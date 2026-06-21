/*
 * SPE Expert 1K-FA LVGL LCD renderer for model-specific screen fragments.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/spe_expert1k/lcd_renderer.h"

#include <Arduino.h>
#include <lvgl.h>
#include <ui.h>

#include "console/serial_console.h"

#define COUNT_OF_LOCAL(a) ((int)(sizeof(a) / sizeof((a)[0])))

#ifndef SPE_VERBOSE_PACKET_LOG
#define SPE_VERBOSE_PACKET_LOG 0
#endif

static const char * const cats[] = {"SPE", "ICOM", "KENWD", "YAESU", "TTEC", "FLEX", "RS232", "NONE"};
static const char * const cat_icom[] = {"CI-V", "VOLTAGE_BAND"};
static const char * const cat_yaesu[] = {"FT 100", "FT 757 GX2", "FT 817/847", "FT 840/890", "FT 897",
                                         "FT 900", "FT 920", "FT 990", "FT 1000", "FT 1000 MP1",
                                         "FT 1000 MP2", "FT 1000 MP3", "FT 2000", "FT 9000D", "BAND_DATA"};
static const char * const cat_tentec[] = {"OMNI VII", "ORION I/II", "JUPITER", "ARGONAUT"};
static const char * const inputs[] = {"1", "2"};
static const char * const ordinals[] = {"1st", "2nd", "3rd", "4th"};
static const char * const ant_num[] = {" 1", " 2", " 3", " 4", "NO"};
static const char * const antenna_heading = " SET ANTENNA vs. IN %s ";
static const char * const antenna_select_footer = "[<^] [v>]:SELECT          [SET]:CHANGE";
static const char * const antenna_save_footer = "[<^] [v>]:SELECT         [SET]:CONFIRM";
static const char * const ant_messages[] = {
  "--- Set %s ANTENNA FOR 160 m BAND ---",
  "---- Set %s ANTENNA FOR 80 m BAND ---",
  "---- Set %s ANTENNA FOR 40 m BAND ---",
  "---- Set %s ANTENNA FOR 30 m BAND ---",
  "---- Set %s ANTENNA FOR 20 m BAND ---",
  "---- Set %s ANTENNA FOR 17 m BAND ---",
  "---- Set %s ANTENNA FOR 15 m BAND ---",
  "---- Set %s ANTENNA FOR 12 m BAND ---",
  "---- Set %s ANTENNA FOR 10 m BAND ---",
  "---- Set %s ANTENNA FOR 6 m BAND ----",
  "------- SAVE SETTINGS AND EXIT -------"
};
static const double capacitor_weights_pf[10] = {5.2, 6.5, 12.5, 21.0, 40.9, 86.5, 168.0, 342.0, 693.5, 1384.5};

static constexpr int meter_bar_x = 136;
static constexpr int meter_bar_width = 420;
static constexpr int meter_scale_label_width = 70;
static constexpr int meter_scale_label_y = 2;
static constexpr int meter_tick_y = 14;
static constexpr int meter_lower_row_offset_y = 0;
static constexpr int meter_tick_count = 21;
static lv_obj_t *ui_powerScaleLabels[5] = {};
static lv_obj_t *ui_paScaleLabels[5] = {};
static lv_obj_t *ui_powerScaleTicks[meter_tick_count] = {};
static lv_obj_t *ui_paScaleTicks[meter_tick_count] = {};

static lv_obj_t *create_meter_scale_tick(lv_obj_t *parent, uint8_t index, int y_offset)
{
  const bool major_tick = (index % 5) == 0;
  lv_obj_t *tick = lv_obj_create(parent);
  lv_obj_remove_style_all(tick);
  lv_obj_set_width(tick, 1);
  lv_obj_set_height(tick, major_tick ? 8 : 4);
  lv_obj_set_x(tick, meter_bar_x + (meter_bar_width * index / (meter_tick_count - 1)));
  lv_obj_set_y(tick, meter_tick_y + y_offset + (major_tick ? 0 : 4));
  lv_obj_set_style_bg_color(tick, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(tick, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  return tick;
}

static lv_obj_t *create_meter_scale_label(lv_obj_t *parent, uint8_t index, int y_offset)
{
  lv_obj_t *label = lv_label_create(parent);
  const int tick_x = meter_bar_x + (meter_bar_width * index / 4);
  lv_obj_set_width(label, meter_scale_label_width);
  lv_obj_set_x(label, tick_x - (meter_scale_label_width / 2));
  lv_obj_set_y(label, meter_scale_label_y + y_offset);
  lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
  return label;
}

static void ensure_transmit_scale_labels()
{
  for (uint8_t i = 0; i < 5; ++i) {
    if (ui_powerScaleLabels[i] == nullptr) {
      ui_powerScaleLabels[i] = create_meter_scale_label(ui_txPowerContainer, i, 0);
    }
    if (ui_paScaleLabels[i] == nullptr) {
      ui_paScaleLabels[i] = create_meter_scale_label(ui_txVoltageContainer, i, meter_lower_row_offset_y);
    }
  }

  for (uint8_t i = 0; i < meter_tick_count; ++i) {
    if (ui_powerScaleTicks[i] == nullptr) {
      ui_powerScaleTicks[i] = create_meter_scale_tick(ui_txPowerContainer, i, 0);
    }
    if (ui_paScaleTicks[i] == nullptr) {
      ui_paScaleTicks[i] = create_meter_scale_tick(ui_txVoltageContainer, i, meter_lower_row_offset_y);
    }
  }
}

void spe_expert1k_configure_transmit_meters(const AmpStatusSnapshot &status)
{
  ensure_transmit_scale_labels();
  lv_label_set_text(ui_powerLabel, status.power_meter.label);
  lv_label_set_text(ui_vPALabel, status.pa_meter.label);
  lv_bar_set_range(ui_powerBar, 0, status.power_meter.max);
  lv_bar_set_range(ui_vBar, 0, status.pa_meter.max);
  for (uint8_t i = 0; i < 5; ++i) {
    lv_label_set_text(ui_powerScaleLabels[i], status.power_meter.scale[i]);
    lv_label_set_text(ui_paScaleLabels[i], status.pa_meter.scale[i]);
  }
}

void spe_expert1k_update_cat_screen(const Expert_Packet &packet)
{
  lv_obj_remove_flag(ui_catStatus, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(ui_ampStatus, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(ui_catType1, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(ui_catType2, LV_OBJ_FLAG_HIDDEN);

  uint8_t s0 = packet.setup[0] & 0x0F;
  uint8_t s1 = packet.setup[1] & 0x0F;
  uint8_t s3 = packet.setup[3] & 0x0F;
  uint8_t s4 = packet.setup[4] & 0x0F;

  if (s0 >= COUNT_OF_LOCAL(cats)) {
    s0 = 0;
  }
  if (s3 >= COUNT_OF_LOCAL(cats)) {
    s3 = 0;
  }
  if (s1 >= COUNT_OF_LOCAL(cat_icom)) {
    s1 = 0;
  }
  if (s4 >= COUNT_OF_LOCAL(cat_icom)) {
    s4 = 0;
  }

  const char *type1 = nullptr;
  const char *type2 = nullptr;

  switch (s0) {
    case 0x01: type1 = cat_icom[s1]; break;
    case 0x03: type1 = cat_yaesu[s1]; break;
    case 0x04: type1 = cat_tentec[s1]; break;
    default: lv_obj_add_flag(ui_catType1, LV_OBJ_FLAG_HIDDEN); break;
  }

  switch (s3) {
    case 0x01: type2 = cat_icom[s4]; break;
    case 0x03: type2 = cat_yaesu[s4]; break;
    case 0x04: type2 = cat_tentec[s4]; break;
    default: lv_obj_add_flag(ui_catType2, LV_OBJ_FLAG_HIDDEN); break;
  }

  lv_label_set_text_fmt(ui_catStatus1, " CAT: %s", cats[s0]);
  lv_label_set_text_fmt(ui_catStatus2, " CAT: %s", cats[s3]);

  if (!lv_obj_has_flag(ui_catType1, LV_OBJ_FLAG_HIDDEN)) {
    lv_label_set_text_fmt(ui_catType1, "TYPE: %s", type1 ? type1 : "");
  }

  if (!lv_obj_has_flag(ui_catType2, LV_OBJ_FLAG_HIDDEN)) {
    lv_label_set_text_fmt(ui_catType2, "TYPE: %s", type2 ? type2 : "");
  }

  lv_label_set_text_fmt(ui_version, " VER:%d%d_%d%d_%d%d_%c",
                        (packet.setup[6] >> 4) & 0x0f, packet.setup[6] & 0x0f,
                        (packet.setup[7] >> 4) & 0x0f, packet.setup[7] & 0x0f,
                        (packet.setup[8] >> 4) & 0x0f, packet.setup[8] & 0x0f,
                        (char)packet.setup[9]);
}

static const char *antenna_label(uint8_t raw)
{
  if (raw >= COUNT_OF_LOCAL(ant_num)) {
    raw = COUNT_OF_LOCAL(ant_num) - 1;
  }
  return ant_num[raw];
}

void spe_expert1k_show_antenna_setup_screen(const Expert_Packet &packet,
                                            lv_obj_t * const antenna_items[],
                                            size_t antenna_item_count)
{
  lv_obj_remove_flag(ui_setupAntOptions, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text_fmt(ui_setupAntHeaderText, antenna_heading, inputs[packet.band_input & 0x01]);

  for (int band = 0; band < 10; ++band) {
    for (int ord = 0; ord < 2; ++ord) {
      const size_t item_index = static_cast<size_t>((band * 2) + ord);
      if (item_index < antenna_item_count && antenna_items[item_index] != nullptr) {
        const uint8_t raw = (packet.setup[band + 1] >> (ord * 4)) & 0x07;
        lv_label_set_text(antenna_items[item_index], antenna_label(raw));
      }
    }
  }
}

uint8_t spe_expert1k_update_antenna_setup_screen(const Expert_Packet &packet,
                                                 lv_obj_t * const antenna_items[],
                                                 size_t antenna_item_count,
                                                 uint8_t previous_selection)
{
  uint8_t index = packet.setup[0];
  if (index > 20) {
    index = 20;
  }

  uint8_t idx = index / 2;
  if (idx >= COUNT_OF_LOCAL(ant_messages)) {
    idx = COUNT_OF_LOCAL(ant_messages) - 1;
  }

  uint8_t ord = index % 2;
  if (ord >= COUNT_OF_LOCAL(ordinals)) {
    ord = 0;
  }

  lv_label_set_text_fmt(ui_setupAntFooterText, ant_messages[idx], ordinals[ord]);
  if (index < 20) {
    if (index < antenna_item_count && antenna_items[index] != nullptr) {
      const uint8_t raw = (packet.setup[idx + 1] >> (ord * 4)) & 0x07;
      lv_label_set_text(antenna_items[index], antenna_label(raw));
    }
    if (previous_selection == 20) {
      lv_label_set_text(ui_setupAntBottomLabel, antenna_select_footer);
    }
  } else {
    lv_label_set_text(ui_setupAntBottomLabel, antenna_save_footer);
  }

  return index;
}

void spe_expert1k_update_manual_tune_screen(const Expert_Packet &packet)
{
  lv_label_set_text_fmt(ui_manualTuneFreq, "%*.3f MHz", 6, float(packet.freq) / 1000.0);
  lv_label_set_text_fmt(ui_manualTuneSubBand, "%*d", 3, packet.sub_band);

  lv_label_set_text_fmt(ui_manualTuneuHLabel, "%*.1f uH", 7, float(packet.setup[1]) / 10.0);
  lv_bar_set_value(ui_manualTuneuH, packet.setup[1], LV_ANIM_ON);

  const uint8_t lo = packet.setup[2];
  const uint8_t hi = packet.setup[3] & 0x03;
  const uint16_t raw = (static_cast<uint16_t>(hi) << 8) | lo;

#if SPE_VERBOSE_PACKET_LOG
  {
    DebugSerialLock debug_lock;
    Serial.print("Low:");
    Serial.print(lo);
    Serial.print(" High:");
    Serial.println(hi);

    for (int i = 9; i >= 0; --i) {
      Serial.print((raw >> i) & 1);
    }
    Serial.println();
  }
#endif

  double pF = 0.0;
  for (int i = 0; i < 10; ++i) {
    if (raw & (1u << i)) {
      pF += capacitor_weights_pf[i];
    }
  }

  lv_label_set_text_fmt(ui_manualTunepFLabel, "%*.1f pF", 7, pF);
  lv_bar_set_value(ui_manualTunepF, pF, LV_ANIM_ON);
}
