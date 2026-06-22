/*
 * Runtime for newer SPE Expert amplifier families using the documented CSV protocol.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/spe_modern/runtime.h"

#include <Arduino.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "amp_dtr.h"
#include "app_status.h"
#include "models/spe_expert1k/packet_parser.h"
#include "models/spe_expert1k/serial_link.h"

static const uint8_t MODERN_CMD_INPUT = 0x01;
static const uint8_t MODERN_CMD_BAND_DOWN = 0x02;
static const uint8_t MODERN_CMD_BAND_UP = 0x03;
static const uint8_t MODERN_CMD_ANTENNA = 0x04;
static const uint8_t MODERN_CMD_L_MINUS = 0x05;
static const uint8_t MODERN_CMD_L_PLUS = 0x06;
static const uint8_t MODERN_CMD_C_MINUS = 0x07;
static const uint8_t MODERN_CMD_C_PLUS = 0x08;
static const uint8_t MODERN_CMD_TUNE = 0x09;
static const uint8_t MODERN_CMD_OFF = 0x0A;
static const uint8_t MODERN_CMD_POWER = 0x0B;
static const uint8_t MODERN_CMD_DISPLAY = 0x0C;
static const uint8_t MODERN_CMD_OPERATE = 0x0D;
static const uint8_t MODERN_CMD_CAT = 0x0E;
static const uint8_t MODERN_CMD_LEFT = 0x0F;
static const uint8_t MODERN_CMD_RIGHT = 0x10;
static const uint8_t MODERN_CMD_SET = 0x11;
static const uint8_t MODERN_CMD_RCU_ON = 0x80;
static const uint8_t MODERN_CMD_RCU_OFF = 0x81;
// ExpertPacketParser strips the AA AA AA 6A stream prefix before handing the
// 367-byte RCU payload here. The documented full-frame LCD body offset is 9;
// inside this stripped payload it is 5.
static const uint16_t MODERN_LCD_BODY_OFFSET = 5;
static const uint8_t MODERN_LCD_COLS = 40;
static const uint8_t MODERN_LCD_ROWS = 8;
static const uint16_t MODERN_LCD_BODY_LEN = MODERN_LCD_COLS * MODERN_LCD_ROWS;
static const uint16_t MODERN_LCD_ATTR_OFFSET = MODERN_LCD_BODY_OFFSET + MODERN_LCD_BODY_LEN;

static const char * const MODERN_BANDS[] = {
  "160 m", "80 m", "60 m", "40 m", "30 m", "20 m",
  "17 m", "15 m", "12 m", "10 m", "6 m", "4 m"
};

static void copy_trimmed(char *dst, size_t dst_len, const char *src, size_t src_len)
{
  if (!dst || dst_len == 0) {
    return;
  }

  while (src_len > 0 && isspace(static_cast<unsigned char>(*src))) {
    ++src;
    --src_len;
  }
  while (src_len > 0 && isspace(static_cast<unsigned char>(src[src_len - 1]))) {
    --src_len;
  }

  const size_t copy_len = src_len < dst_len - 1 ? src_len : dst_len - 1;
  memcpy(dst, src, copy_len);
  dst[copy_len] = '\0';
}

static float parse_float_field(const char *value)
{
  return value ? static_cast<float>(atof(value)) : 0.0f;
}

static uint16_t parse_uint_field(const char *value)
{
  return value ? static_cast<uint16_t>(atoi(value)) : 0;
}

static char decode_lcd_char(uint8_t value)
{
  if (value == 0x00) {
    return ' ';
  }
  if (value == 0x8f) {
    return '|';
  }
  if (value == 0xaa) {
    return '*';
  }
  if (value >= 0x01 && value <= 0x5f) {
    return static_cast<char>(value + 0x20);
  }
  if (value >= 0x20 && value <= 0x7e) {
    return static_cast<char>(value);
  }
  return ' ';
}

static void rtrim(char *text)
{
  size_t len = strlen(text);
  while (len > 0 && text[len - 1] == ' ') {
    text[--len] = '\0';
  }
}

static void decode_lcd_segment(char *dst,
                               size_t dst_len,
                               const uint8_t *src,
                               uint16_t offset,
                               uint8_t rows,
                               uint8_t cols)
{
  if (!dst || dst_len == 0) {
    return;
  }
  dst[0] = '\0';

  size_t out = 0;
  for (uint8_t row = 0; row < rows && out + 1 < dst_len; ++row) {
    char line[41] = {};
    const uint8_t line_cols = cols < sizeof(line) ? cols : static_cast<uint8_t>(sizeof(line) - 1);
    for (uint8_t col = 0; col < line_cols; ++col) {
      line[col] = decode_lcd_char(src[offset + row * cols + col]);
    }
    line[line_cols] = '\0';
    rtrim(line);

    if (row > 0 && out + 1 < dst_len) {
      dst[out++] = '\n';
    }
    for (const char *p = line; *p && out + 1 < dst_len; ++p) {
      dst[out++] = *p;
    }
  }
  dst[out] = '\0';
}

static void decode_lcd_rows(char *dst, size_t dst_len, const uint8_t *src)
{
  decode_lcd_segment(dst, dst_len, src, MODERN_LCD_BODY_OFFSET, MODERN_LCD_ROWS, MODERN_LCD_COLS);
}

static char hex_digit(uint8_t value)
{
  value &= 0x0f;
  return value < 10 ? static_cast<char>('0' + value) : static_cast<char>('A' + value - 10);
}

static void encode_hex_bytes(char *dst, size_t dst_len, const uint8_t *src, uint16_t offset, uint16_t count, uint16_t len)
{
  if (!dst || dst_len == 0) {
    return;
  }
  dst[0] = '\0';

  size_t out = 0;
  for (uint16_t i = 0; i < count && out + 2 < dst_len; ++i) {
    uint8_t value = 0x00;
    if (src && offset + i < len) {
      value = src[offset + i];
    }
    dst[out++] = hex_digit(value >> 4);
    dst[out++] = hex_digit(value);
  }
  dst[out] = '\0';
}

static bool text_contains_ignore_case(const char *haystack, const char *needle)
{
  if (!haystack || !needle || !*needle) {
    return false;
  }

  const size_t needle_len = strlen(needle);
  for (const char *p = haystack; *p; ++p) {
    size_t i = 0;
    while (i < needle_len && p[i] &&
           toupper(static_cast<unsigned char>(p[i])) == toupper(static_cast<unsigned char>(needle[i]))) {
      ++i;
    }
    if (i == needle_len) {
      return true;
    }
  }
  return false;
}

SpeModernRuntime::SpeModernRuntime(AmpModelId model_id)
  : model_id_(model_id)
{
}

void SpeModernRuntime::set_model_id(AmpModelId model_id)
{
  model_id_ = model_id;
}

AmpModelId SpeModernRuntime::model_id() const
{
  return model_id_;
}

bool SpeModernRuntime::status_valid() const
{
  return status_valid_;
}

uint8_t SpeModernRuntime::screen_id() const
{
  return screen_id_;
}

const char *SpeModernRuntime::screen_name() const
{
  return screen_name_;
}

unsigned long SpeModernRuntime::last_activity_ms() const
{
  return last_activity_ms_;
}

void SpeModernRuntime::publish_status() const
{
  publish_current_status();
}

bool SpeModernRuntime::press_key(const char *name)
{
  if (!name) {
    return false;
  }

  if (strcmp(name, "l_down") == 0) {
    return send_key_command(MODERN_CMD_L_MINUS);
  } else if (strcmp(name, "l_up") == 0) {
    return send_key_command(MODERN_CMD_L_PLUS);
  } else if (strcmp(name, "c_down") == 0) {
    return send_key_command(MODERN_CMD_C_MINUS);
  } else if (strcmp(name, "c_up") == 0) {
    return send_key_command(MODERN_CMD_C_PLUS);
  } else if (strcmp(name, "tune") == 0) {
    return send_key_command(MODERN_CMD_TUNE);
  } else if (strcmp(name, "input") == 0) {
    return send_key_command(MODERN_CMD_INPUT);
  } else if (strcmp(name, "band_down") == 0) {
    return send_key_command(MODERN_CMD_BAND_DOWN);
  } else if (strcmp(name, "band_up") == 0) {
    return send_key_command(MODERN_CMD_BAND_UP);
  } else if (strcmp(name, "ant") == 0) {
    return send_key_command(MODERN_CMD_ANTENNA);
  } else if (strcmp(name, "left") == 0) {
    return send_key_command(MODERN_CMD_LEFT);
  } else if (strcmp(name, "right") == 0) {
    return send_key_command(MODERN_CMD_RIGHT);
  } else if (strcmp(name, "cat") == 0) {
    return send_key_command(MODERN_CMD_CAT);
  } else if (strcmp(name, "set") == 0) {
    return send_key_command(MODERN_CMD_SET);
  } else if (strcmp(name, "off") == 0) {
    amp_dtr_set(false);
    remote_update_enabled_ = false;
    if (!send_command(MODERN_CMD_OFF)) {
      return false;
    }
    return send_command(MODERN_CMD_RCU_OFF);
  } else if (strcmp(name, "on") == 0) {
    return power_on();
  } else if (strcmp(name, "power") == 0) {
    return send_key_command(MODERN_CMD_POWER);
  } else if (strcmp(name, "display") == 0) {
    return send_key_command(MODERN_CMD_DISPLAY);
  } else if (strcmp(name, "operate") == 0) {
    return send_key_command(MODERN_CMD_OPERATE);
  }

  return false;
}

bool SpeModernRuntime::power_on()
{
  amp_dtr_set(true);
  remote_update_enabled_ = true;
  if (!send_command(MODERN_CMD_RCU_OFF)) {
    return false;
  }
  return send_command(MODERN_CMD_RCU_ON);
}

bool SpeModernRuntime::refresh_remote()
{
  if (!send_command(MODERN_CMD_RCU_OFF)) {
    return false;
  }
  return send_command(MODERN_CMD_RCU_ON);
}

bool SpeModernRuntime::remote_updates_enabled() const
{
  return remote_update_enabled_;
}

void SpeModernRuntime::process_next_queued_command()
{
  spe_expert1k_process_next_queued_command();
}

void SpeModernRuntime::mark_activity(unsigned long now)
{
  last_activity_ms_ = now;
}

bool SpeModernRuntime::should_send_keepalive(unsigned long now, unsigned long interval_ms) const
{
  return now - last_poll_ms_ >= interval_ms;
}

void SpeModernRuntime::note_keepalive(unsigned long now)
{
  last_poll_ms_ = now;
}

bool SpeModernRuntime::send_command(uint8_t opcode)
{
  return spe_expert1k_queue_command({opcode});
}

bool SpeModernRuntime::send_key_command(uint8_t opcode)
{
  if (!send_command(opcode)) {
    return false;
  }
  return refresh_remote();
}

uint32_t SpeModernRuntime::rcu_frame_signature(const uint8_t *data, uint16_t len) const
{
  if (!data) {
    return 0;
  }

  uint32_t hash = 2166136261UL;
  for (uint16_t i = MODERN_LCD_BODY_OFFSET; i < len; ++i) {
    hash ^= data[i];
    hash *= 16777619UL;
  }
  return hash ? hash : 1;
}

void SpeModernRuntime::process_status_packet(const uint8_t *data, uint8_t len, unsigned long now)
{
  if (!data || len != 67) {
    return;
  }

  char line[68] = {};
  memcpy(line, data, 67);
  line[67] = '\0';

  const char *fields[21] = {};
  uint8_t field_count = 0;
  fields[field_count++] = line;
  for (char *p = line; *p && field_count < 21; ++p) {
    if (*p == ',') {
      *p = '\0';
      fields[field_count++] = p + 1;
    }
  }
  if (field_count < 21) {
    return;
  }

  const char *model = fields[1];
  if (strcmp(model, "13K") == 0) {
    model_id_ = AmpModelId::SpeExpert13k;
  } else if (strcmp(model, "15K") == 0) {
    model_id_ = AmpModelId::SpeExpert15k;
  } else if (strcmp(model, "20K") == 0) {
    model_id_ = AmpModelId::SpeExpert2k;
  } else if (model_id_ == AmpModelId::Unknown) {
    model_id_ = AmpModelId::SpeModern;
  }

  const char *op = fields[2];
  const char *tx = fields[3];
  const char *input = fields[5];
  const char *band = fields[6];
  const char *tx_ant = fields[7];
  const char *out = fields[9];
  const char *power = fields[10];
  const char *swr = fields[11];
  const char *voltage = fields[13];
  const char *current = fields[14];
  const char *temp = fields[15];

  copy_trimmed(input_, sizeof(input_), input, strlen(input));
  const uint8_t band_index = static_cast<uint8_t>(atoi(band));
  if (band_index < sizeof(MODERN_BANDS) / sizeof(MODERN_BANDS[0])) {
    copy_trimmed(band_, sizeof(band_), MODERN_BANDS[band_index], strlen(MODERN_BANDS[band_index]));
  } else {
    copy_trimmed(band_, sizeof(band_), band, strlen(band));
  }
  copy_trimmed(antenna_, sizeof(antenna_), tx_ant, tx_ant && *tx_ant ? 1 : 0);
  copy_trimmed(out_, sizeof(out_), out, strlen(out));
  copy_trimmed(swr_, sizeof(swr_), swr, strlen(swr));
  snprintf(temp_, sizeof(temp_), "%uC", parse_uint_field(temp));

  power_ = parse_float_field(power);
  reverse_ = 0.0f;
  voltage_ = parse_float_field(voltage);
  current_ = parse_float_field(current);
  op_status_ = (op && *op == 'O') ? 1 : 0;
  screen_id_ = (tx && *tx == 'T') ? 2 : op_status_;
  copy_trimmed(screen_name_, sizeof(screen_name_), op_status_ ? "operate" : "standby", op_status_ ? 7 : 7);

  status_valid_ = true;
  last_activity_ms_ = now;
  publish_current_status();
}

void SpeModernRuntime::process_rcu_frame(const uint8_t *data, uint16_t len, unsigned long now)
{
  if (!data || len < MODERN_RCU_PAYLOAD_LEN) {
    return;
  }

  const uint32_t frame_signature = rcu_frame_signature(data, len);
  last_activity_ms_ = now;
  remote_update_enabled_ = true;
  if (status_valid_ && frame_signature == last_rcu_frame_signature_) {
    return;
  }
  last_rcu_frame_signature_ = frame_signature;

  decode_lcd_segment(lcd_title_, sizeof(lcd_title_), data, MODERN_LCD_BODY_OFFSET, 1, MODERN_LCD_COLS);
  decode_lcd_rows(lcd_body_, sizeof(lcd_body_), data);
  encode_hex_bytes(lcd_cells_, sizeof(lcd_cells_), data, MODERN_LCD_BODY_OFFSET, MODERN_LCD_BODY_LEN, len);
  encode_hex_bytes(lcd_attrs_, sizeof(lcd_attrs_), data, MODERN_LCD_ATTR_OFFSET, MODERN_LCD_COLS, len);
  decode_lcd_segment(lcd_footer_, sizeof(lcd_footer_), data, MODERN_LCD_BODY_OFFSET + MODERN_LCD_COLS * 6, 1, MODERN_LCD_COLS);
  decode_lcd_segment(lcd_hint_, sizeof(lcd_hint_), data, MODERN_LCD_BODY_OFFSET + MODERN_LCD_COLS * 7, 1, MODERN_LCD_COLS);

  if (text_contains_ignore_case(lcd_body_, "STANDBY")) {
    screen_id_ = 0;
    copy_trimmed(screen_name_, sizeof(screen_name_), "standby", 7);
  } else if (text_contains_ignore_case(lcd_body_, "PA OUT") || text_contains_ignore_case(lcd_title_, "PA OUT")) {
    screen_id_ = 1;
    copy_trimmed(screen_name_, sizeof(screen_name_), "operate", 7);
  } else if (text_contains_ignore_case(lcd_title_, "SETUP")) {
    screen_id_ = 3;
    copy_trimmed(screen_name_, sizeof(screen_name_), "setup_options", 13);
  } else if (text_contains_ignore_case(lcd_title_, "SET ANTENNA")) {
    screen_id_ = 4;
    copy_trimmed(screen_name_, sizeof(screen_name_), "set_antenna", 11);
  } else if (text_contains_ignore_case(lcd_title_, "CAT")) {
    screen_id_ = 5;
    copy_trimmed(screen_name_, sizeof(screen_name_), "cat", 3);
  } else if (text_contains_ignore_case(lcd_title_, "TUNEABLE ANTENNAS")) {
    screen_id_ = 6;
    copy_trimmed(screen_name_, sizeof(screen_name_), "tun_ant_port", 12);
  } else if (text_contains_ignore_case(lcd_title_, "SYSTEM INFO")) {
    screen_id_ = 7;
    copy_trimmed(screen_name_, sizeof(screen_name_), "system_info", 11);
  } else if (lcd_title_[0]) {
    screen_id_ = 8;
    copy_trimmed(screen_name_, sizeof(screen_name_), lcd_title_, strlen(lcd_title_));
  } else {
    screen_id_ = 0;
    copy_trimmed(screen_name_, sizeof(screen_name_), "modern", 6);
  }

  status_valid_ = true;
  publish_current_status();
}

void SpeModernRuntime::publish_current_status() const
{
  AppStatusSnapshot snapshot;
  snapshot.valid = status_valid_;
  snapshot.amp.valid = status_valid_;
  snapshot.amp.model = model_id_;
  snapshot.amp.screen_id = screen_id_;
  snapshot.amp.screen_name = screen_name_;
  snapshot.amp.op_status = op_status_;
  snapshot.amp.input = input_;
  snapshot.amp.band = band_;
  snapshot.amp.antenna = antenna_;
  snapshot.amp.cat = cat_;
  snapshot.amp.out = out_;
  strncpy(snapshot.amp.swr, swr_, sizeof(snapshot.amp.swr) - 1);
  strncpy(snapshot.amp.temp, temp_, sizeof(snapshot.amp.temp) - 1);
  snapshot.amp.lcd_title = lcd_title_;
  snapshot.amp.lcd_body = lcd_body_;
  snapshot.amp.lcd_cells = lcd_cells_;
  snapshot.amp.lcd_attrs = lcd_attrs_;
  snapshot.amp.lcd_footer = lcd_footer_;
  snapshot.amp.lcd_hint = lcd_hint_;
  snapshot.amp.power = power_;
  snapshot.amp.reverse = reverse_;
  snapshot.amp.voltage = voltage_;
  snapshot.amp.current = current_;
  snapshot.amp.power_meter.label = "PW OUT";
  snapshot.amp.power_meter.value = power_;
  snapshot.amp.power_meter.max = model_id_ == AmpModelId::SpeExpert13k ? 1300 : model_id_ == AmpModelId::SpeExpert2k ? 2000 : 1500;
  snapshot.amp.power_meter.suffix = "W pep";
  snapshot.amp.power_meter.scale[0] = "0";
  snapshot.amp.power_meter.scale[1] = "";
  snapshot.amp.power_meter.scale[2] = "";
  snapshot.amp.power_meter.scale[3] = "";
  snapshot.amp.power_meter.scale[4] = "";
  snapshot.amp.pa_meter.label = "V PA";
  snapshot.amp.pa_meter.value = voltage_;
  snapshot.amp.pa_meter.max = 60;
  snapshot.amp.pa_meter.suffix = "V";
  snapshot.amp.pa_meter.scale[0] = "0";
  snapshot.amp.pa_meter.scale[1] = "15";
  snapshot.amp.pa_meter.scale[2] = "30";
  snapshot.amp.pa_meter.scale[3] = "45";
  snapshot.amp.pa_meter.scale[4] = "60";

  snapshot.model_data.model = model_id_;
  snapshot.model_data.screen_id = screen_id_;
  snapshot.model_data.screen_name = screen_name_;
  app_status_publish(snapshot);
}
