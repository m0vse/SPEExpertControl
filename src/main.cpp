/*
 * Main application entry point, FreeRTOS task setup, amplifier packet handling, and LCD UI state updates.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include <Arduino.h>
#include <rtos.h>

#include "Arduino_GigaDisplayTouch.h"
#include "amp_control.h"
#include "app_status.h"
#include "display/lvgl_giga_display.h"
#include "display/lvgl_giga_touch.h"
#include "lvgl.h"
#include <ui.h>
#include "models/spe_expert1k/expertpackets.h"
#include "models/spe_expert1k/menuitems.h"
#include "ui/menu_control.h"

#if SPE_ENABLE_WEB_SERVER
#include "network/control_server.h"
#endif

#if SPE_ENABLE_WIFI_SETUP
#include "ui/wifi_setup.h"
#endif

#if SPE_ENABLE_WIFI_SETUP || SPE_ENABLE_WEB_SERVER
#include <WiFi.h>
#endif

#include <initializer_list>
#include <string.h>

using namespace std::chrono_literals;

#ifndef SPE_BRINGUP_LEVEL
#define SPE_BRINGUP_LEVEL 5
#endif

#ifndef SPE_VERBOSE_PACKET_LOG
#define SPE_VERBOSE_PACKET_LOG 0
#endif

#ifndef SPE_AMP_DTR_PIN
#define SPE_AMP_DTR_PIN 7
#endif

#ifndef SPE_AMP_DTR_ASSERTED_LEVEL
#define SPE_AMP_DTR_ASSERTED_LEVEL LOW
#endif

uint8_t progress = 0;

#define COUNT_OF(a) ((int)(sizeof(a) / sizeof((a)[0])))

void process_packet(const Expert_Packet &packet_in, uint8_t len_in);
void send_command(std::initializer_list<uint8_t> cmd);
void button_pressed(lv_event_t * e);
void ui_task(void);
void serial_task(void);

const char* outs[] = {"HALF", "FULL"};

const char* tscales[] = {"°F", "°C"};

const char* startup[] = {"Standby", "Operate"};

const char* onoff[] = {"Off", "On "};

const char* inputs[] = {"1", "2"};

const char* ordinals[] = {"1st","2nd","3rd","4th"};

const char* antennas[] = {"1","2","3","4"};

const char* bands[] = {"160 m","80 m","40 m","30 m","20 m","17 m","15 m","12 m","10 m","6 m"};

const char* cats[] = {"SPE","ICOM","KENWD","YAESU","TTEC","FLEX","RS232","NONE" };

const char* cat_icom[] = {"CI-V","VOLTAGE_BAND"};

const char* cat_yaesu[] = {"FT 100","FT 757 GX2","FT 817/847","FT 840/890","FT 897","FT 900","FT 920","FT 990","FT 1000","FT 1000 MP1",
                            "FT 1000 MP2","FT 1000 MP3","FT 2000","FT 9000D","BAND_DATA"};

const char* cat_tentec[] = {"OMNI VII","ORION I/II","JUPITER","ARGONAUT"};

const char* warnings[] = {"0x10: DEBUGGING (IGNORE)",        "POWER MANAGEMENT : V PA < 20 V", 
                          "POWER MANAGEMENT : V PA < 26 V",  "POWER MANAGEMENT : V PA > 50 V",
                          "POWER MANAGEMENT : V PA > 50 V",  "POWER MANAGEMENT : I PA > 40 A",  
                          "POWER MANAGEMENT : I PA > 50 A",  "OVER TEMPERATURE : TEMP > 90°C",
                          "P.A. MANAGEMENT : OVER DRIVING",  "0x19: DEBUGGING (IGNORE)",
                          "0x20: DEBUGGING (IGNORE)",        "P.A.MANAGEMENT : PW REV > 300W",  
                          "P.A. MANAGEMENT : PA PROTECTION" };
                          
const char* headings[] = { 
          " SETUP OPTIONS vs. IN %s ", 
          " SET ANTENNA vs. IN %s ",
          " SET CAT vs. IN %s ",
          " SET YAESU vs. IN %s ",
          " SET ICOM vs. IN %s ",
          " SET TEN-TEC vs. IN %s ",
          " SET BAUD RATE vs. IN %s ",
          "[<^] [v>]:SELECT          [SET]:CHANGE",
          "[<^] [v>]:SELECT         [SET]:CONFIRM"
};

const char* baud_rate[] = {"1200 Baud","2400 Baud","4800 Baud","9600 Baud"};

const char* setup_options[] = {"ANTENNA","CAT","MANUAL TUNE","BACKLIGHT","CONTEST","BEEP","START","TEMP","QUIT"};

const char* setup_messages[] = {
  "------- SET ANTENNAS vs. BANDS -------",
  "----- SET CAT INTERFACE FEATURES -----",
  "------- MANUAL TUNE OPERATIONS -------",
  "----- DISPLAY BACKLIGHT SETTINGS -----",
  "-------- CONTEST MODE On/Off ---------",
  "------ ACOUSTIC FEEDBACK On/Off ------",
  "------ SET STARTUP DEFAULT MODE ------",
  "-- TEMPERATURE VALUE SHOWN IN °C/°F --",
  "---------- LEAVE THIS MENU -----------"
};

const char* ant_messages[] = {
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

const char* ant_num[] = { " 1", " 2", " 3", " 4","NO" };

enum ExpertStatus {Sync,Len,Data,Sum};

enum ExpertCommands {Key_On=0x10,Rcu_On=0x80,Rcu_Off=0x81,Cat_232=0x82};

enum ExpertKeys {L_Minus_Key=0x30,L_Plus_Key=0x31,C_Minus_Key=0x32,C_Plus_Key=0x33,Tune_Key=0x34,In_Key=0x28,Band_Minus_Key=0x29,Band_Plus_Key=0x2A,
                Ant_Key=0x2B,Cat_Key=0x2C,Left_Key=0x2D,Right_Key=0x2E,Set_Key=0x2F,Off_Key=0x18,Power_Key=0x1A,Display_Key=0x1B,Operate_Key=0x1C};

enum ExpertScreen {Receive_Screen=0x00,Operate_RX,Operate_TX,Cat_Screen,Data_Stored,Unused_Screen_A,Setup_Options,Set_Antenna,Set_Cat,Set_Yaesu,Set_Icom,Set_TenTec,
                  Set_BaudRate,Manual_Tune,Backlight,Unused_Screen_B,Unused_Screen_C,
                  Warning_V_Low_Half,Warning_V_Low_Full,Warning_V_High_Half,Warning_V_High_Full,Warning_A_High_Half,Warning_A_High_Full,Warning_Temp,Warning_Over_Driving,
                  Unused_Screen_D,Unused_Screen_E,Warning_Reverse,Warning_Protection,
                  Alarm_History=0x1D,Shutdown=0x1E,BootMessage=0xff};

class ExpertPacketParser {
public:
  enum class Result {
    None,
    PacketReady,
    InvalidChecksum
  };

  Result read(uint8_t value)
  {
    switch (state_) {
      case Sync:
        if (value != 0xAA) {
          bytes_ = 0x00;
        } else if (bytes_ == 2) {
          state_ = Len;
        } else {
          bytes_++;
        }
        break;
      case Len:
        if (value <= MAX_DATA) {
          length_ = value;
          state_ = Data;
        } else {
          reset();
        }
        bytes_ = 0x00;
        break;
      case Data:
        reinterpret_cast<uint8_t *>(&packet_)[bytes_++] = value;
        checksum_ += value;
        if (bytes_ == length_) {
          state_ = Sum;
        }
        break;
      case Sum:
        if (checksum_ == value) {
          resetState();
          return Result::PacketReady;
        }
        reset();
        return Result::InvalidChecksum;
    }

    return Result::None;
  }

  const Expert_Packet &packet() const { return packet_; }
  uint8_t length() const { return length_; }

private:
  void reset()
  {
    resetState();
    length_ = 0x00;
  }

  void resetState()
  {
    bytes_ = 0x00;
    checksum_ = 0x00;
    state_ = Sync;
  }

  ExpertStatus state_ = Sync;
  Expert_Packet packet_{};
  uint8_t bytes_ = 0x00;
  uint8_t checksum_ = 0x00;
  uint8_t length_ = 0x00;
};

// Setup options menu items
//static lv_obj_t *setup_options_items[] {ui_setupAntenna,ui_setupCat,ui_setupManualTune,ui_setupBacklight,ui_setupContest,ui_setupBeep,ui_setupStart,ui_setupTemp,ui_setupQuit};
MenuController setup_options_ctrl;
static lv_obj_t *setup_options_items[9];

MenuController setup_ant_ctrl;
static lv_obj_t *setup_ant_items[21];

MenuController setup_cat_ctrl;
static lv_obj_t *setup_cat_items[8];

MenuController setup_yaesu_ctrl;
static lv_obj_t *setup_yaesu_items[15];

MenuController setup_icom_ctrl;
static lv_obj_t *setup_icom_items[2];

MenuController setup_tentec_ctrl;
static lv_obj_t *setup_tentec_items[4];

MenuController setup_baudrate_ctrl;
static lv_obj_t *setup_baudrate_items[4];

ExpertScreen screen = BootMessage;

// Amplifier settings
Expert_Packet last_status;
unsigned long last_rcu=0;
const unsigned long interval = 1000;
bool hold = false;
bool touch_ready = false;
bool amp_status_valid = false;
static bool amp_dtr_asserted = false;
static bool amp_remote_update_enabled = true;
int transformed_touch_devices = 0;
unsigned long last_console_status = 0;
static char console_line[96];
static uint8_t console_line_len = 0;
static bool console_poll_enabled = false;
static bool console_last_was_line_end = false;
static rtos::Mutex lvgl_mutex;
static rtos::Mutex amp_serial_mutex;
static rtos::Mutex debug_serial_mutex;
static rtos::Thread ui_thread(osPriorityNormal, 8192, nullptr, "ui");
static rtos::Thread serial_thread(osPriorityNormal, 8192, nullptr, "amp");

class LvglLock {
public:
  LvglLock() { lvgl_mutex.lock(); }
  ~LvglLock() { lvgl_mutex.unlock(); }
};

class AmpSerialLock {
public:
  AmpSerialLock() { amp_serial_mutex.lock(); }
  ~AmpSerialLock() { amp_serial_mutex.unlock(); }
};

class DebugSerialLock {
public:
  DebugSerialLock() { debug_serial_mutex.lock(); }
  ~DebugSerialLock() { debug_serial_mutex.unlock(); }
};

class AmpSerialLink {
public:
  void begin()
  {
    AmpSerialLock lock;
    Serial1.begin(9600);
  }

  int available()
  {
    AmpSerialLock lock;
    return Serial1.available();
  }

  bool read(ExpertPacketParser::Result &result)
  {
    int value = -1;
    {
      AmpSerialLock lock;
      if (Serial1.available() <= 0) {
        return false;
      }
      value = Serial1.read();
    }

    result = parser_.read(static_cast<uint8_t>(value));
    return true;
  }

  const Expert_Packet &packet() const { return parser_.packet(); }
  uint8_t length() const { return parser_.length(); }

  void send(std::initializer_list<uint8_t> cmd)
  {
    AmpSerialLock lock;
    Serial1.write(0x55); Serial1.write(0x55); Serial1.write(0x55);
    Serial1.write(cmd.size() & 0xff);
    uint8_t sum = 0;
    for (uint8_t c : cmd) {
      Serial1.write(c);
      sum += c;
    }
    Serial1.write(sum);

#if SPE_VERBOSE_PACKET_LOG
    DebugSerialLock debug_lock;
    Serial.print(F("TX command:"));
    for (uint8_t c : cmd) {
      Serial.print(' ');
      Serial.print(c, HEX);
    }
    Serial.println();
#endif
  }

private:
  ExpertPacketParser parser_;
};

static AmpSerialLink amp_serial;

static void amp_dtr_set(bool asserted)
{
  if (asserted) {
    digitalWrite(SPE_AMP_DTR_PIN, SPE_AMP_DTR_ASSERTED_LEVEL);
  } else {
    digitalWrite(SPE_AMP_DTR_PIN, SPE_AMP_DTR_ASSERTED_LEVEL == HIGH ? LOW : HIGH);
  }
  amp_dtr_asserted = asserted;
}

static void amp_dtr_begin()
{
  pinMode(SPE_AMP_DTR_PIN, OUTPUT);
  amp_dtr_set(true);
}

static void boot_log(const __FlashStringHelper *message)
{
  Serial.println(message);
  Serial.flush();
}

static void boot_led_set(uint8_t pin, bool on)
{
  digitalWrite(pin, on ? LOW : HIGH);
}

static void boot_stage(uint8_t stage, const __FlashStringHelper *message)
{
  Serial.print(F("Boot stage "));
  Serial.print(stage);
  Serial.print(F(": "));
  Serial.println(message);
  Serial.flush();

#ifdef SPE_BOOT_DIAGNOSTICS
  for (uint8_t i = 0; i < stage; ++i) {
    boot_led_set(LEDB, true);
    delay(160);
    boot_led_set(LEDB, false);
    delay(160);
  }
  delay(900);
#endif
}

static const __FlashStringHelper *wifi_status_name(int status)
{
  switch (status) {
    case WL_IDLE_STATUS: return F("idle");
    case WL_NO_SSID_AVAIL: return F("no_ssid");
    case WL_SCAN_COMPLETED: return F("scan_completed");
    case WL_CONNECTED: return F("connected");
    case WL_CONNECT_FAILED: return F("connect_failed");
    case WL_CONNECTION_LOST: return F("connection_lost");
    case WL_DISCONNECTED: return F("disconnected");
    case WL_NO_MODULE: return F("no_module");
    default: return F("unknown");
  }
}

static const char *screen_name(ExpertScreen value)
{
  switch (value) {
    case Receive_Screen: return "receive";
    case Operate_RX: return "operate_rx";
    case Operate_TX: return "operate_tx";
    case Cat_Screen: return "cat";
    case Data_Stored: return "data_stored";
    case Setup_Options: return "setup_options";
    case Set_Antenna: return "set_antenna";
    case Set_Cat: return "set_cat";
    case Set_Yaesu: return "set_yaesu";
    case Set_Icom: return "set_icom";
    case Set_TenTec: return "set_tentec";
    case Set_BaudRate: return "set_baudrate";
    case Manual_Tune: return "manual_tune";
    case Backlight: return "backlight";
    case Alarm_History: return "alarm_history";
    case Shutdown: return "shutdown";
    case BootMessage: return "boot";
    default: return "other";
  }
}

static void print_console_help()
{
  DebugSerialLock debug_lock;
  Serial.println(F("Commands:"));
  Serial.println(F("  help        Show this help"));
  Serial.println(F("  status      Controller summary"));
  Serial.println(F("  wifi        WiFi status, IP and firmware"));
  Serial.println(F("  amp         Last amplifier status packet"));
  Serial.println(F("  scan        Blocking WiFi scan to serial"));
  Serial.println(F("  rcu         Send RCU_ON to the amplifier"));
  Serial.println(F("  dtr         Show amplifier DTR output state"));
  Serial.println(F("  wifi-popup  Open WiFi popup on the display"));
  Serial.println(F("  wifi-saved  Show saved WiFi credential state"));
  Serial.println(F("  wifi-clear  Clear saved WiFi credentials"));
  Serial.println(F("  poll on     Enable periodic status polling"));
  Serial.println(F("  poll off    Disable periodic status polling"));
}

static void print_wifi_status()
{
  DebugSerialLock debug_lock;
#if SPE_ENABLE_WIFI_SETUP || SPE_ENABLE_WEB_SERVER
  const int status = WiFi.status();
  Serial.print(F("WiFi status="));
  Serial.print(status);
  Serial.print(F(" ("));
  Serial.print(wifi_status_name(status));
  Serial.println(F(")"));
  Serial.print(F("WiFi firmware="));
  Serial.println(WiFi.firmwareVersion());
  Serial.print(F("WiFi setup connected="));
#if SPE_ENABLE_WIFI_SETUP
  Serial.println(wifi_setup_is_connected() ? F("yes") : F("no"));
  Serial.print(F("WiFi saved credentials="));
  Serial.println(wifi_setup_has_saved_credentials() ? F("yes") : F("no"));
#else
  Serial.println(F("n/a"));
#endif
  if (status == WL_CONNECTED) {
    Serial.print(F("SSID="));
    Serial.println(WiFi.SSID());
    Serial.print(F("IP="));
    Serial.println(WiFi.localIP());
    Serial.print(F("RSSI="));
    Serial.print(WiFi.RSSI());
    Serial.println(F(" dBm"));
  }
#else
  Serial.println(F("WiFi disabled at build time"));
#endif
}

static void print_amp_status()
{
  DebugSerialLock debug_lock;
  Serial.print(F("Amp status valid="));
  Serial.println(amp_status_valid ? F("yes") : F("no"));
  Serial.print(F("screen="));
  Serial.print(static_cast<int>(screen));
  Serial.print(F(" ("));
  Serial.print(screen_name(screen));
  Serial.println(F(")"));
  Serial.print(F("last_rcu_ms="));
  Serial.println(last_rcu);
  Serial.print(F("dtr_pin=D"));
  Serial.print(SPE_AMP_DTR_PIN);
  Serial.print(F(" asserted="));
  Serial.print(amp_dtr_asserted ? F("yes") : F("no"));
  Serial.print(F(" gpio_level="));
  Serial.println(digitalRead(SPE_AMP_DTR_PIN) == HIGH ? F("HIGH") : F("LOW"));

  if (!amp_status_valid) {
    Serial.println(F("No 30-byte amplifier status packet received yet"));
    return;
  }

  const uint8_t band_idx = (last_status.band_input >> 4) & 0x0f;
  const uint8_t input_idx = last_status.band_input & 0x01;
  const uint8_t ant_idx = last_status.antenna_cat & 0x07;
  const uint8_t cat_idx = (last_status.antenna_cat >> 4) & 0x07;
  const uint8_t out_idx = (last_status.flags >> 4) & 0x01;

  Serial.print(F("band="));
  Serial.print(band_idx < COUNT_OF(bands) ? bands[band_idx] : "?");
  Serial.print(F(" input="));
  Serial.print(input_idx < COUNT_OF(inputs) ? inputs[input_idx] : "?");
  Serial.print(F(" antenna="));
  Serial.print(ant_idx < COUNT_OF(antennas) ? antennas[ant_idx] : "?");
  Serial.print(F(" cat="));
  Serial.print(cat_idx < COUNT_OF(cats) ? cats[cat_idx] : "?");
  Serial.print(F(" out="));
  Serial.println(out_idx < COUNT_OF(outs) ? outs[out_idx] : "?");

  Serial.print(F("power="));
  Serial.print(float(last_status.power) / 10);
  Serial.print(F("W rev="));
  Serial.print(float(last_status.rev_power) / 10);
  Serial.print(F("W swr/gain="));
  Serial.print(float(last_status.swr_gain) / 10);
  Serial.print(F(" temp="));
  Serial.print(last_status.temp);
  Serial.print(F(" voltage="));
  Serial.print(float(last_status.voltage) / 10);
  Serial.print(F(" current="));
  Serial.println(float(last_status.current) / 10);
}

static void json_print_string(Print &out, const char *value)
{
  out.print('"');
  if (value) {
    for (const char *p = value; *p; ++p) {
      if (*p == '"' || *p == '\\') {
        out.print('\\');
      }
      out.print(*p);
    }
  }
  out.print('"');
}

void app_status_print_json(Print &out)
{
  out.print(F("{\"valid\":"));
  out.print(amp_status_valid ? F("true") : F("false"));
  out.print(F(",\"screen\":"));
  out.print(static_cast<int>(screen));
  out.print(F(",\"screenName\":"));
  json_print_string(out, screen_name(screen));

  if (!amp_status_valid) {
    out.print(F("}"));
    return;
  }

  const uint8_t band_idx = (last_status.band_input >> 4) & 0x0f;
  const uint8_t input_idx = last_status.band_input & 0x01;
  const uint8_t ants_idx = last_status.antenna_cat & 0x07;
  const uint8_t cat_idx = (last_status.antenna_cat >> 4) & 0x07;
  const uint8_t out_idx = (last_status.flags >> 4) & 0x01;
  const bool current_mode = ((last_status.flags >> 2) & 0x01) != 0;
  const bool swr_alarm = ((last_status.flags >> 3) & 0x01) != 0;

  out.print(F(",\"flags\":"));
  out.print(last_status.flags);
  out.print(F(",\"displayCtx\":"));
  out.print(last_status.display_ctx);
  out.print(F(",\"freq\":"));
  out.print(last_status.freq);
  out.print(F(",\"subBand\":"));
  out.print(last_status.sub_band);
  out.print(F(",\"setup\":["));
  for (uint8_t i = 0; i < COUNT_OF(last_status.setup); ++i) {
    if (i) {
      out.print(',');
    }
    out.print(last_status.setup[i]);
  }
  out.print(']');
  out.print(F(",\"input\":"));
  json_print_string(out, input_idx < COUNT_OF(inputs) ? inputs[input_idx] : "?");
  out.print(F(",\"band\":"));
  json_print_string(out, band_idx < COUNT_OF(bands) ? bands[band_idx] : "?");
  out.print(F(",\"antenna\":"));
  json_print_string(out, ants_idx < COUNT_OF(antennas) ? antennas[ants_idx] : "?");
  out.print(F(",\"cat\":"));
  json_print_string(out, cat_idx < COUNT_OF(cats) ? cats[cat_idx] : "?");
  out.print(F(",\"out\":"));
  json_print_string(out, out_idx < COUNT_OF(outs) ? outs[out_idx] : "?");
  out.print(F(",\"power\":"));
  out.print(float(last_status.power) / 10.0f, 1);
  out.print(F(",\"reverse\":"));
  out.print(float(last_status.rev_power) / 10.0f, 1);
  out.print(F(",\"swr\":"));
  if (swr_alarm) {
    json_print_string(out, "--.--");
  } else {
    char swr[8];
    snprintf(swr, sizeof(swr), "%.2f", float(last_status.swr_gain) / 100.0f);
    json_print_string(out, swr);
  }
  out.print(F(",\"temp\":"));
  char temp[12];
  snprintf(temp, sizeof(temp), "%d%s", last_status.temp, tscales[(last_status.flags >> 7) & 0x01]);
  json_print_string(out, temp);
  out.print(F(",\"voltage\":"));
  out.print(float(last_status.voltage) / 10.0f, 1);
  out.print(F(",\"current\":"));
  out.print(current_mode ? float(last_status.voltage) / 10.0f : float(last_status.current) / 10.0f, 1);
  out.print(F("}"));
}

static void print_controller_status()
{
  DebugSerialLock debug_lock;
  Serial.print(F("Controller ms="));
  Serial.print(millis());
  Serial.print(F(" progress="));
  Serial.print(progress);
  Serial.print(F(" touch="));
  Serial.print(touch_ready ? F("ok") : F("failed"));
  Serial.print(F(" indev="));
  Serial.print(transformed_touch_devices);
  Serial.print(F(" screen="));
  Serial.print(static_cast<int>(screen));
  Serial.print(F(" ("));
  Serial.print(screen_name(screen));
  Serial.println(F(")"));
#if SPE_BRINGUP_LEVEL >= 5
  Serial.print(F("serial1_available="));
  Serial.println(amp_serial.available());
#endif
}

static void print_wifi_scan()
{
#if SPE_ENABLE_WIFI_SETUP || SPE_ENABLE_WEB_SERVER
  {
    DebugSerialLock debug_lock;
    Serial.println(F("Starting blocking WiFi scan..."));
  }
  const int networks = WiFi.scanNetworks();
  {
    DebugSerialLock debug_lock;
    Serial.print(F("WiFi networks found="));
    Serial.println(networks);
    for (int i = 0; i < networks; ++i) {
      Serial.print(i);
      Serial.print(F(": "));
      Serial.print(WiFi.SSID(i));
      Serial.print(F(" RSSI="));
      Serial.print(WiFi.RSSI(i));
      Serial.print(F(" enc="));
      Serial.println(WiFi.encryptionType(i));
    }
  }
#else
  DebugSerialLock debug_lock;
  Serial.println(F("WiFi disabled at build time"));
#endif
}

static void handle_console_command(char *line)
{
  while (*line == ' ' || *line == '\t') {
    ++line;
  }

  for (char *p = line; *p; ++p) {
    if (*p >= 'A' && *p <= 'Z') {
      *p = *p - 'A' + 'a';
    }
  }

  if (line[0] == '\0') {
    return;
  }
  if (strcmp(line, "help") == 0 || strcmp(line, "?") == 0) {
    print_console_help();
  } else if (strcmp(line, "status") == 0) {
    print_controller_status();
  } else if (strcmp(line, "wifi") == 0) {
    print_wifi_status();
  } else if (strcmp(line, "amp") == 0) {
    print_amp_status();
  } else if (strcmp(line, "scan") == 0) {
    print_wifi_scan();
  } else if (strcmp(line, "rcu") == 0) {
    send_command({Rcu_On});
  } else if (strcmp(line, "dtr") == 0) {
    DebugSerialLock debug_lock;
    Serial.print(F("Amp DTR pin=D"));
    Serial.print(SPE_AMP_DTR_PIN);
    Serial.print(F(" asserted="));
    Serial.print(amp_dtr_asserted ? F("yes") : F("no"));
    Serial.print(F(" gpio_level="));
    Serial.println(digitalRead(SPE_AMP_DTR_PIN) == HIGH ? F("HIGH") : F("LOW"));
  } else if (strcmp(line, "wifi-popup") == 0) {
#if SPE_ENABLE_WIFI_SETUP
    LvglLock lock;
    wifi_setup_set_visible(true);
#else
    DebugSerialLock debug_lock;
    Serial.println(F("WiFi setup disabled at build time"));
#endif
  } else if (strcmp(line, "wifi-saved") == 0) {
#if SPE_ENABLE_WIFI_SETUP
    DebugSerialLock debug_lock;
    wifi_setup_print_saved_credentials();
#else
    DebugSerialLock debug_lock;
    Serial.println(F("WiFi setup disabled at build time"));
#endif
  } else if (strcmp(line, "wifi-clear") == 0) {
#if SPE_ENABLE_WIFI_SETUP
    DebugSerialLock debug_lock;
    wifi_setup_clear_saved_credentials();
#else
    DebugSerialLock debug_lock;
    Serial.println(F("WiFi setup disabled at build time"));
#endif
  } else if (strcmp(line, "poll on") == 0) {
    console_poll_enabled = true;
    DebugSerialLock debug_lock;
    Serial.println(F("Periodic status polling enabled"));
  } else if (strcmp(line, "poll off") == 0) {
    console_poll_enabled = false;
    DebugSerialLock debug_lock;
    Serial.println(F("Periodic status polling disabled"));
  } else {
    DebugSerialLock debug_lock;
    Serial.print(F("Unknown command: "));
    Serial.println(line);
    Serial.println(F("Type help for commands"));
  }
}

static void console_service(void)
{
  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\r' || c == '\n') {
      if (console_last_was_line_end) {
        continue;
      }
      console_last_was_line_end = true;
      console_line[console_line_len] = '\0';
      handle_console_command(console_line);
      console_line_len = 0;
      continue;
    }
    console_last_was_line_end = false;
    if (c == '\b' || c == 0x7f) {
      if (console_line_len > 0) {
        --console_line_len;
      }
      continue;
    }
    if (console_line_len < sizeof(console_line) - 1) {
      console_line[console_line_len++] = c;
    }
  }

  if (!console_poll_enabled) {
    return;
  }

  unsigned long now = millis();
  if (now - last_console_status < 2000) {
    return;
  }

  last_console_status = now;
#if SPE_BRINGUP_LEVEL >= 5
  int serial1_available = 0;
  {
    serial1_available = amp_serial.available();
  }
#endif

  DebugSerialLock debug_lock;
  Serial.print(F("Status: ms="));
  Serial.print(now);
  Serial.print(F(" screen="));
  Serial.print(static_cast<int>(screen));
  Serial.print(F(" progress="));
  Serial.print(progress);
  Serial.print(F(" touch="));
  Serial.print(touch_ready ? F("ok") : F("failed"));
  Serial.print(F(" indev="));
  Serial.print(transformed_touch_devices);
  Serial.print(F(" serial1_available="));
#if SPE_BRINGUP_LEVEL >= 5
  Serial.println(serial1_available);
#else
  Serial.println(0);
#endif
}

void setup() {
  Serial.begin(115200); // Debug logging
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  boot_led_set(LEDR, false);
  boot_led_set(LEDG, false);
  boot_led_set(LEDB, false);
  amp_dtr_begin();

  while(!Serial && millis()<4000) {
    delay(10);
  }

  boot_log(F("Boot: serial ready"));
  Serial.print(F("SPE bring-up level "));
  Serial.println(SPE_BRINGUP_LEVEL);
  Serial.flush();

#if SPE_BRINGUP_LEVEL == 0
  boot_log(F("Stable baseline: display, touch, UI, WiFi and amp serial disabled"));
  boot_led_set(LEDG, true);
  return;
#endif

#if SPE_BRINGUP_LEVEL >= 1
  boot_stage(1, F("display begin"));
  int display_status = giga_lvgl_display_begin();
  Serial.print(F("Boot: display status "));
  Serial.println(display_status);
  Serial.flush();
  if (display_status != 0) {
    boot_log(F("Boot: display init failed; stopping before LVGL UI"));
    boot_led_set(LEDR, true);
    while (true) {
      delay(1000);
    }
  }
#endif

#if SPE_BRINGUP_LEVEL >= 2
  boot_stage(3, F("construct touch"));
  static Arduino_GigaDisplayTouch TouchDetector;

  boot_stage(4, F("touch begin"));
  touch_ready = TouchDetector.begin();
  transformed_touch_devices = giga_lvgl_touch_use_display_rotation(TouchDetector);
  Serial.print(F("Boot: touch status "));
  Serial.println(touch_ready ? F("ok") : F("failed"));
  Serial.print(F("Boot: transformed touch devices "));
  Serial.println(transformed_touch_devices);
#endif

#if SPE_BRINGUP_LEVEL >= 3
  boot_stage(5, F("ui init"));
  ui_init();
#endif

#if SPE_BRINGUP_LEVEL >= 4
  boot_stage(6, F("setup menus"));
  setupMenus();
#endif

#if SPE_BRINGUP_LEVEL >= 5
  boot_stage(7, F("serial1 begin"));
  amp_serial.begin(); // Amp connection
#endif

  Serial.println(F("Starting Expert1K controller"));

#if SPE_BRINGUP_LEVEL >= 4
  // Setup options menu items (must be done after ui_init())
  setup_options_items[0] = ui_setupAntenna;
  setup_options_items[1] = ui_setupCat;
  setup_options_items[2] = ui_setupManualTune;
  setup_options_items[3] = ui_setupBacklight;
  setup_options_items[4] = ui_setupContest;
  setup_options_items[5] = ui_setupBeep;
  setup_options_items[6] = ui_setupStart;
  setup_options_items[7] = ui_setupTemp;
  setup_options_items[8] = ui_setupQuit;
  setup_options_ctrl.begin(setup_options_items, COUNT_OF(setup_options_items));

  setup_ant_items[0] = ui_setup160Ant1;
  setup_ant_items[1] = ui_setup160Ant2;
  setup_ant_items[2] = ui_setup80Ant1;
  setup_ant_items[3] = ui_setup80Ant2;
  setup_ant_items[4] = ui_setup40Ant1;
  setup_ant_items[5] = ui_setup40Ant2;
  setup_ant_items[6] = ui_setup30Ant1;
  setup_ant_items[7] = ui_setup30Ant2;
  setup_ant_items[8] = ui_setup20Ant1;
  setup_ant_items[9] = ui_setup20Ant2;
  setup_ant_items[10] = ui_setup17Ant1;
  setup_ant_items[11] = ui_setup17Ant2;
  setup_ant_items[12] = ui_setup15Ant1;
  setup_ant_items[13] = ui_setup15Ant2;
  setup_ant_items[14] = ui_setup12Ant1;
  setup_ant_items[15] = ui_setup12Ant2;
  setup_ant_items[16] = ui_setup10Ant1;
  setup_ant_items[17] = ui_setup10Ant2;
  setup_ant_items[18] = ui_setup6Ant1;
  setup_ant_items[19] = ui_setup6Ant2;
  setup_ant_items[20] = ui_setupAntSave;
  setup_ant_ctrl.begin(setup_ant_items, COUNT_OF(setup_ant_items));

  setup_cat_items[0] = ui_setupCatSpe;
  setup_cat_items[1] = ui_setupCatIcom;
  setup_cat_items[2] = ui_setupCatKenwood;
  setup_cat_items[3] = ui_setupCatYaesu;
  setup_cat_items[4] = ui_setupCatTenTec;
  setup_cat_items[5] = ui_setupCatFlexRadio;
  setup_cat_items[6] = ui_setupCatRs232;
  setup_cat_items[7] = ui_setupCatNone;
  setup_cat_ctrl.begin(setup_cat_items, COUNT_OF(setup_cat_items));

  setup_yaesu_items[0] = ui_setupYaesuFT100;
  setup_yaesu_items[1] = ui_setupYaesuFT757;
  setup_yaesu_items[2] = ui_setupYaesuFT817;
  setup_yaesu_items[3] = ui_setupYaesuFT840;
  setup_yaesu_items[4] = ui_setupYaesuFT897;
  setup_yaesu_items[5] = ui_setupYaesuFT900;
  setup_yaesu_items[6] = ui_setupYaesuFT920;
  setup_yaesu_items[7] = ui_setupYaesuFT990;
  setup_yaesu_items[8] = ui_setupYaesuFT1000;
  setup_yaesu_items[9] = ui_setupYaesuFT1000MP1;
  setup_yaesu_items[10] = ui_setupYaesuFT1000MP2;
  setup_yaesu_items[11] = ui_setupYaesuFT1000MP3;
  setup_yaesu_items[12] = ui_setupYaesuFT2000;
  setup_yaesu_items[13] = ui_setupYaesuFT9000;
  setup_yaesu_items[14] = ui_setupYaesuBandData;
  setup_yaesu_ctrl.begin(setup_yaesu_items, COUNT_OF(setup_yaesu_items));

  setup_icom_items[0] = ui_setupIcomCiv;
  setup_icom_items[1] = ui_setupIcomVoltage;
  setup_icom_ctrl.begin(setup_icom_items, COUNT_OF(setup_icom_items));

  setup_tentec_items[0] = ui_setupTenTecOmni;
  setup_tentec_items[1] = ui_setupTenTecOrion;
  setup_tentec_items[2] = ui_setupTenTecJupiter;
  setup_tentec_items[3] = ui_setupTenTecArgonaut;
  setup_tentec_ctrl.begin(setup_tentec_items, COUNT_OF(setup_tentec_items));

  setup_baudrate_items[0] = ui_setupBaud1200;
  setup_baudrate_items[1] = ui_setupBaud2400;
  setup_baudrate_items[2] = ui_setupBaud4800;
  setup_baudrate_items[3] = ui_setupBaud9600;
  setup_baudrate_ctrl.begin(setup_baudrate_items, COUNT_OF(setup_baudrate_items));

  last_rcu=millis();
  lv_disp_load_scr(ui_bootScreen);
  lv_timer_handler();

#if SPE_ENABLE_WIFI_SETUP
  wifi_setup_create();
#endif
#endif

#if SPE_BRINGUP_LEVEL >= 3
  ui_thread.start(ui_task);
#endif

#if SPE_BRINGUP_LEVEL >= 5
  serial_thread.start(serial_task);
#endif

}

void loop()
{
  rtos::ThisThread::sleep_for(1s);
}

void ui_task()
{
  while (true) {
#if SPE_BRINGUP_LEVEL >= 3
    {
      LvglLock lock;
      lv_timer_handler();

#if SPE_ENABLE_WIFI_SETUP && SPE_BRINGUP_LEVEL >= 4
      wifi_setup_service();
#endif
    }
#endif

    rtos::ThisThread::sleep_for(5ms);
  }
}

void serial_task()
{
  while (true) {
#if SPE_ENABLE_WEB_SERVER && SPE_BRINGUP_LEVEL >= 5
    #if SPE_ENABLE_WIFI_SETUP
    if (wifi_setup_is_connected()) {
      control_server_service();
    }
    #else
    control_server_service();
    #endif
#endif

  // Check for serial data
  while (true) {
    ExpertPacketParser::Result read_result = ExpertPacketParser::Result::None;
    if (!amp_serial.read(read_result)) {
      break;
    }

    switch (read_result) {
      case ExpertPacketParser::Result::PacketReady:
        process_packet(amp_serial.packet(), amp_serial.length());
        break;
      case ExpertPacketParser::Result::InvalidChecksum:
      {
        DebugSerialLock debug_lock;
        Serial.println("Invalid checksum");
        break;
      }
      case ExpertPacketParser::Result::None:
        break;
    }
  }  

  console_service();

// Send Rcu_On command if nothing received for over ten seconds.
  unsigned long now = millis();
  if (amp_remote_update_enabled && now - last_rcu >= interval)
  {
    //Serial1.end();      // close serial port
    //delay(100);        //wait 100 millis
    //Serial1.begin(9600);
    send_command({Rcu_On});
    last_rcu=now;
    if (progress < 100) {
      progress++;
    }
    {
      LvglLock lock;
      lv_bar_set_value(ui_startupBar, progress, LV_ANIM_OFF);
    }
  }

    rtos::ThisThread::sleep_for(5ms);
  }
}

void process_packet(const Expert_Packet &packet_in, uint8_t len_in)
{
  // We have a valid packet!
  if (len_in == 1) {
    // This is a response packet
    //if (packet_in.status_code == 0x06)
    //Serial.println("ACK");
    if (packet_in.status_code == 0x15) {
      DebugSerialLock debug_lock;
      Serial.println("NAK");
    } else if (packet_in.status_code == 0xff) {
      DebugSerialLock debug_lock;
      Serial.println("UNK");
    } else if (packet_in.status_code != 0x06) {
      DebugSerialLock debug_lock;
      Serial.print(F("Unexpected 1-byte response: 0x"));
      Serial.println(packet_in.status_code, HEX);
    }
  } else if (len_in == 30) {
    // Show status
    amp_status_valid = true;
    last_rcu=millis();
    LvglLock lock;

    // Any valid status packet means the amp serial link is alive.
    if (screen == BootMessage) {
#if SPE_ENABLE_WIFI_SETUP
        wifi_setup_set_visible(false);
#endif
        lv_disp_load_scr(ui_mainScreen);
    }

    // Only upate if packet has changed
    if (memcmp(&last_status,&packet_in,sizeof last_status))
    {
      // Select current screen
      // Receive_Screen=0x00,Operate_RX,Operate_TX,Cat_Screen,UnusedA,Data_Stored,Setup_Options,Set_Antenna,Set_Cat,Set_Yaesu,Set_Icom,Set_TenTec,
      // Set_BaudRate,Manual_Tune,Backlight,UnusedB,UnusedC,Alarm_History,Shutdown
      ExpertScreen scr = static_cast<ExpertScreen>(packet_in.display_ctx);
      
      if ((screen != scr || packet_in.flags != last_status.flags) && !hold ) {
        // Screen has changed, so lets display it, hide everything first.
        // Need to tidy this, probably create an array of all screen objects?
        lv_obj_add_flag(ui_receive, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_ampStatus, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_catStatus, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_manualTune, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_backlight, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_transmit, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_alarmHistory, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_alarmControl, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_warningScreen, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_warningControl, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupOptions, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_systemMessage, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupAntOptions, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupCatOptions, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupYaesuOptions, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupIcomOptions, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupTenTecOptions, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupBaudRateOptions, LV_OBJ_FLAG_HIDDEN); 
        switch (scr) {
          case Receive_Screen:
            if ((packet_in.flags >> 2) & 0x01) {
              // TX mode
              lv_obj_remove_flag(ui_transmit, LV_OBJ_FLAG_HIDDEN);
              lv_label_set_text(ui_powerLabel,"  OUT ");
              lv_obj_add_flag(ui_txVoltageContainer, LV_OBJ_FLAG_HIDDEN); 
            } else {
              lv_obj_remove_flag(ui_receive, LV_OBJ_FLAG_HIDDEN);
              lv_obj_remove_flag(ui_txVoltageContainer, LV_OBJ_FLAG_HIDDEN); 
            }
            lv_obj_remove_flag(ui_ampStatus, LV_OBJ_FLAG_HIDDEN);
            break;
          case Cat_Screen:
          {
            hold=true;
            lv_obj_remove_flag(ui_catStatus, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui_ampStatus, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui_catType1, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui_catType2, LV_OBJ_FLAG_HIDDEN);

            uint8_t s0 = packet_in.setup[0] & 0x0F;
            uint8_t s1 = packet_in.setup[1] & 0x0F;
            uint8_t s3 = packet_in.setup[3] & 0x0F;
            uint8_t s4 = packet_in.setup[4] & 0x0F;

            // clamp
            if (s0 >= COUNT_OF(cats))      s0 = 0;
            if (s3 >= COUNT_OF(cats))      s3 = 0;
            if (s1 >= COUNT_OF(cat_icom))  s1 = 0;
            if (s4 >= COUNT_OF(cat_icom))  s4 = 0;  // or use appropriate array for the path

            // Now select
            const char *type1 = nullptr;
            const char *type2 = nullptr;

            switch (s0) {
              case 0x01: type1 = cat_icom[s1];  break;
              case 0x03: type1 = cat_yaesu[s1]; break;
              case 0x04: type1 = cat_tentec[s1]; break;
              default:   lv_obj_add_flag(ui_catType1, LV_OBJ_FLAG_HIDDEN); break;
            }

            switch (s3) {
              case 0x01: type2 = cat_icom[s4];  break;
              case 0x03: type2 = cat_yaesu[s4]; break;
              case 0x04: type2 = cat_tentec[s4]; break;
              default:   lv_obj_add_flag(ui_catType2, LV_OBJ_FLAG_HIDDEN); break;
            }

            lv_label_set_text_fmt(ui_catStatus1, " CAT: %s", cats[s0]);
            lv_label_set_text_fmt(ui_catStatus2, " CAT: %s", cats[s3]);

            if (!lv_obj_has_flag(ui_catType1, LV_OBJ_FLAG_HIDDEN))
                lv_label_set_text_fmt(ui_catType1, "TYPE: %s", type1 ? type1 : "");

            if (!lv_obj_has_flag(ui_catType2, LV_OBJ_FLAG_HIDDEN))
                lv_label_set_text_fmt(ui_catType2, "TYPE: %s", type2 ? type2 : "");
            
            lv_label_set_text_fmt(ui_version," VER:%d%d_%d%d_%d%d_%c",
                  (packet_in.setup[6] >> 4) & 0x0f, packet_in.setup[6] & 0x0f,
                  (packet_in.setup[7] >> 4) & 0x0f, packet_in.setup[7] & 0x0f,
                  (packet_in.setup[8] >> 4) & 0x0f, packet_in.setup[8] & 0x0f, (char)packet_in.setup[9]);
            break;
          }
          case Operate_RX:
          case Operate_TX:
            lv_obj_remove_flag(ui_transmit, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui_ampStatus, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui_txVoltageContainer, LV_OBJ_FLAG_HIDDEN); 
            lv_label_set_text(ui_powerLabel,"PA OUT");
            lv_label_set_text(ui_vPALabel,  "  I PA");
            break;
          case Alarm_History:
            lv_obj_remove_flag(ui_alarmHistory, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui_alarmControl, LV_OBJ_FLAG_HIDDEN);
            last_status.setup[0]=0xff; // Make sure it updates!
            break;
          case Setup_Options:
            lv_obj_remove_flag(ui_setupOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupHeaderText,headings[0],inputs[packet_in.band_input & 0x01]);
            break;
          case Set_Antenna:
            lv_obj_remove_flag(ui_setupAntOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupAntHeaderText,headings[1],inputs[packet_in.band_input & 0x01]);
            // Populate all labels
            for (int f=0;f<10;f++)
            {
              for (int g=0;g<2;g++)
              {
                lv_label_set_text(setup_ant_items[(f*2)+g],ant_num[(packet_in.setup[f+1] >> (g*4)) & 0x07]);
              }
            }
            break;
          case Set_Cat:
            lv_obj_remove_flag(ui_setupCatOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupCatHeaderText,headings[2],inputs[packet_in.band_input & 0x01]);
            break;
          case Set_Yaesu:
            lv_obj_remove_flag(ui_setupYaesuOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupIcomHeaderText,headings[3],inputs[packet_in.band_input & 0x01]);
            break;
          case Set_Icom:
            lv_obj_remove_flag(ui_setupIcomOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupIcomHeaderText,headings[4],inputs[packet_in.band_input & 0x01]);
            break;
          case Set_TenTec:
            lv_obj_remove_flag(ui_setupTenTecOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupTenTecHeaderText,headings[5],inputs[packet_in.band_input & 0x01]);
            break;
          case Set_BaudRate:
            lv_obj_remove_flag(ui_setupBaudRateOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupBaudRateHeaderText,headings[6],inputs[packet_in.band_input & 0x01]);
            break;
          case Manual_Tune:
            lv_obj_remove_flag(ui_manualTune, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui_ampStatus, LV_OBJ_FLAG_HIDDEN);
            break;
          case Backlight:
            lv_obj_remove_flag(ui_backlight, LV_OBJ_FLAG_HIDDEN);
            break;
          case Shutdown:
            lv_obj_remove_flag(ui_systemMessage, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ui_systemMessageText,"SHUTDOWN");
            break;
          case Data_Stored:
            lv_obj_remove_flag(ui_systemMessage, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ui_systemMessageText,"DATA STORED");
            break;
          case Warning_V_Low_Half:
          case Warning_V_Low_Full:
          case Warning_V_High_Half:
          case Warning_V_High_Full:
          case Warning_A_High_Half:
          case Warning_A_High_Full:
          case Warning_Temp:
          case Warning_Over_Driving:
          case Warning_Reverse:
          case Warning_Protection:
            lv_obj_remove_flag(ui_warningScreen, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui_warningControl, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_warningText,"%s",warnings[packet_in.display_ctx & 0x0f]);
            break;
          default:
          break;
        }
        screen = scr;
        // If screen has changed, last_status is now invalid;
        memset(&last_status,0xff,sizeof last_status);
      }

      if (scr == Alarm_History && last_status.setup[0] != packet_in.setup[0]) 
      {
        // Refresh warnings as currently displayed has changed.
        uint8_t wrn_idx = (packet_in.setup[0]) >> 4 & 0x0f;
        uint8_t wrn_no = packet_in.setup[0] & 0x0f;
        if (wrn_no && (packet_in.setup[wrn_idx] & 0x0f) < 0x0D)
          lv_label_set_text_fmt(ui_alarmLine1, "%*d)IN %s %s",2,wrn_idx,inputs[(packet_in.setup[wrn_idx] >> 7) & 0x01],warnings[packet_in.setup[wrn_idx] & 0x0f]);
        if (wrn_no > 1 && (packet_in.setup[wrn_idx-1] & 0x0f) < 0x0D)
          lv_label_set_text_fmt(ui_alarmLine2, "%*d)IN %s %s",2,wrn_idx-1,inputs[(packet_in.setup[wrn_idx-1] >> 7) & 0x01],warnings[packet_in.setup[wrn_idx-1] & 0x0f]);
        if (wrn_no > 2 && (packet_in.setup[wrn_idx-2] & 0x0f) < 0x0D)
          lv_label_set_text_fmt(ui_alarmLine3, "%*d)IN %s %s",2,wrn_idx-2,inputs[(packet_in.setup[wrn_idx-2] >> 7) & 0x01],warnings[packet_in.setup[wrn_idx-2] & 0x0f]);
        if (wrn_no > 3 && (packet_in.setup[wrn_idx-3] & 0x0f) < 0x0D)
          lv_label_set_text_fmt(ui_alarmLine4, "%*d)IN %s %s",2,wrn_idx-3,inputs[(packet_in.setup[wrn_idx-3] >> 7) & 0x01],warnings[packet_in.setup[wrn_idx-3] & 0x0f]);
      } 

      if (memcmp(&last_status.setup,&packet_in.setup,sizeof last_status.setup)|| last_status.flags != packet_in.flags)
      {
        // Something has changed!
        if (scr == Setup_Options) 
        {
          // Setup_Options Menu has changed.
          lv_label_set_text_fmt(ui_setupFooterText,"%s",setup_messages[packet_in.setup[1] & 0x0f]);        
          // We need to update the menu options that are changeable
          lv_label_set_text_fmt(ui_setupContest,"CONTEST %s",onoff[(packet_in.flags >> 5) & 0x01]);
          lv_label_set_text_fmt(ui_setupBeep,   "BEEP    %s",onoff[(packet_in.flags >> 6) & 0x01]);
          lv_label_set_text_fmt(ui_setupStart,  "START   %s",startup[(packet_in.flags >> 1) & 0x01]);
          lv_label_set_text_fmt(ui_setupTemp,   "TEMP    %s",tscales[(packet_in.flags >> 7) & 0x01]);

          setup_options_ctrl.applySelection(packet_in.setup[1] & 0x0f);
        }
        else if (scr == Set_Antenna) 
        {
          // Setup_Options Menu has changed.
          // The documentation appears to be incorrect. It suggests that the setup[0] defines the band 
          // and in each other setup bit 7 shows the current selected ant.
          // This does not appear to be the case, as setup[0] goes up to 20!
          uint8_t index = packet_in.setup[0];
          if (index > 20) index = 20;  // clamp to last valid

          uint8_t idx = index / 2;
          if (idx >= COUNT_OF(ant_messages)) idx = COUNT_OF(ant_messages) - 1;

          uint8_t ord = index % 2;
          if (ord >= COUNT_OF(ordinals)) ord = 0; // safety

          uint8_t raw = (packet_in.setup[idx+1] >> (ord*4)) & 0x07;
          if (raw >= COUNT_OF(ant_num)) raw = COUNT_OF(ant_num) - 1;

          lv_label_set_text_fmt(ui_setupAntFooterText,ant_messages[idx],ordinals[ord]); 
          if (index < 20) // Don't change label of SAVE!     
          {
            lv_label_set_text(setup_ant_items[index],ant_num[(packet_in.setup[idx+1] >> (ord*4)) & 0x07]);
            if (setup_ant_ctrl.selected() == 20) { // Was previously SAVE
              lv_label_set_text(ui_setupAntBottomLabel,headings[7]);
            }
          } else {
            lv_label_set_text(ui_setupAntBottomLabel,headings[8]);
          }
          setup_ant_ctrl.applySelection(index);
        }
        else if (scr == Set_Cat) 
        {
          setup_cat_ctrl.applySelection(packet_in.setup[1]);
        }
        else if (scr == Set_Yaesu) 
        {
          setup_yaesu_ctrl.applySelection(packet_in.setup[1]);
        }
        else if (scr == Set_Icom) 
        {
          setup_icom_ctrl.applySelection(packet_in.setup[1]);
        }
        else if (scr == Set_TenTec) 
        {
          setup_tentec_ctrl.applySelection(packet_in.setup[1]);
        }
        else if (scr == Set_BaudRate) 
        {
          setup_baudrate_ctrl.applySelection(packet_in.setup[1]);
        }
        else if (scr == Manual_Tune)
        {
          // Update ManualTune contents
          lv_label_set_text_fmt(ui_manualTuneFreq,"%*.3f MHz",6,float(packet_in.freq)/1000.0);
          lv_label_set_text_fmt(ui_manualTuneSubBand,"%*d",3,packet_in.sub_band);

          lv_label_set_text_fmt(ui_manualTuneuHLabel,"%*.1f uH",7,float(packet_in.setup[1])/10.0);
          lv_bar_set_value(ui_manualTuneuH, packet_in.setup[1], LV_ANIM_ON);




          uint8_t lo = packet_in.setup[2];
          uint8_t hi = packet_in.setup[3] & 0x03;
          uint16_t raw = (static_cast<uint16_t>((hi) << 8) | lo);

#if SPE_VERBOSE_PACKET_LOG
          {
            DebugSerialLock debug_lock;
            Serial.print("Low:");
            Serial.print(lo);
            Serial.print(" High:");
            Serial.println(hi);

            for (int i = 9; i >= 0; --i) Serial.print((raw >> i) & 1);
            Serial.println();
          }
#endif
          
          static const double weights[10] = { 5.2, 6.5, 12.5, 21.0, 40.9, 86.5, 168.0, 342.0, 693.5, 1384.5 };

          double pF = 0.0;
          for (int i = 0; i < 10; ++i) {
              if (raw & (1u << i)) {
                  pF += weights[i];
              }
          }

          lv_label_set_text_fmt(ui_manualTunepFLabel,"%*.1f pF",7,pF);
          lv_bar_set_value(ui_manualTunepF, pF, LV_ANIM_ON);
        }
        else if (scr == Backlight)
        {
          // Update ManualTune contents
          lv_bar_set_value(ui_backlightLevel, packet_in.setup[1], LV_ANIM_ON);
        }
      }
      // No point updating any other ui elements unless they have actually changed since the last update.

      // Standby/Operate screen data

      int8_t band_idx = (packet_in.band_input >> 4) & 0x0f;
      int8_t input_idx = (packet_in.band_input) & 0x01;
      int8_t ants_idx = (packet_in.antenna_cat) & 0x07;
      int8_t cat_idx = (packet_in.antenna_cat >> 4) & 0x07;
      int8_t out_idx = (packet_in.flags >> 4) & 0x01;
      if (band_idx >= COUNT_OF(bands) || input_idx >= COUNT_OF(inputs) || ants_idx >= COUNT_OF(antennas) || cat_idx >= COUNT_OF(cats) || out_idx >= COUNT_OF(outs)) {
            DebugSerialLock debug_lock;
            Serial.println("Value exceeds allowed number, aborting update..");
            return;
      }
    
      if (last_status.band_input != packet_in.band_input) {
        lv_label_set_text_fmt(ui_bandLabel, "BAND\n%s", bands[band_idx]);
        lv_label_set_text_fmt(ui_inLabel,"IN\n%s",inputs[input_idx]);      
      }

      if (last_status.antenna_cat != packet_in.antenna_cat) {
        lv_label_set_text_fmt(ui_antLabel,"ANT\n%s",antennas[ants_idx]); // only 3 bits
        lv_label_set_text_fmt(ui_catLabel,"CAT\n%s",cats[cat_idx]); // only 3 bits     
      }

      if (last_status.flags != packet_in.flags) {
        lv_label_set_text_fmt(ui_outLabel,"OUT\n%s",outs[out_idx]);      
      }

      if (last_status.power != packet_in.power) {
        if (scr == Operate_TX) {
          lv_label_set_text_fmt(ui_pep,"%*.1f W pep",6,float(packet_in.power)/10.0);      
        } else {
          lv_label_set_text_fmt(ui_pep,"%*.1f W pep",6,float(packet_in.power)/10.0);     
        } 
        lv_bar_set_value(ui_powerBar, packet_in.power/10, LV_ANIM_ON);
      }

      if (last_status.voltage != packet_in.voltage) {
        if (scr == Operate_TX) {
          lv_label_set_text_fmt(ui_vPA,"%*.1f A",4,float(packet_in.voltage)/10.0);      
        } else {
          lv_label_set_text_fmt(ui_vPA,"%*.1f v",4,float(packet_in.voltage)/10.0);      
        }
        lv_bar_set_value(ui_vBar, packet_in.voltage/10, LV_ANIM_ON);
      }

      if (last_status.swr_gain != packet_in.swr_gain) {
        if (packet_in.swr_gain == 0)
          lv_label_set_text(ui_swrLabel,"SWR\n--.--");
        else
          lv_label_set_text_fmt(ui_swrLabel,"SWR\n%*.2f",5,float(packet_in.swr_gain)/100.0);
      }

      if (last_status.temp != packet_in.temp) {
        lv_label_set_text_fmt(ui_tempLabel,"TEMP\n%d%s",packet_in.temp,tscales[(packet_in.flags >> 7) & 0x01]);
      }

#if SPE_VERBOSE_PACKET_LOG
      {
        DebugSerialLock debug_lock;
        Serial.print("SWR/Gain:");
        Serial.print(float(packet_in.swr_gain)/10);
        Serial.print(" Temp:");
        Serial.print(packet_in.temp);
        Serial.print(" Power:");
        Serial.print(float(packet_in.power)/10);
        Serial.print(" Reverse:");
        Serial.print(float(packet_in.rev_power)/10);
        Serial.print(" Voltage:");
        Serial.print(float(packet_in.voltage)/10);
        Serial.print(" Current:");
        Serial.println(float(packet_in.current)/10);
      }
#endif
      memcpy(&last_status,&packet_in,sizeof last_status);
    }
  }
}


void send_command(std::initializer_list<uint8_t> cmd) {
  const bool rcu_on = cmd.size() == 1 && *cmd.begin() == Rcu_On;
  const bool off_key = cmd.size() == 2 && *cmd.begin() == Key_On && *(cmd.begin() + 1) == Off_Key;

  if (rcu_on) {
    amp_remote_update_enabled = true;
    amp_dtr_set(true);
  }

  amp_serial.send(cmd);

  if (off_key) {
    amp_serial.send({Rcu_Off});
    amp_remote_update_enabled = false;
    amp_dtr_set(false);
  }
}

bool amp_control_press_key(const char *name)
{
  if (strcmp(name, "l_down") == 0) {
    send_command({Key_On, L_Minus_Key});
  } else if (strcmp(name, "l_up") == 0) {
    send_command({Key_On, L_Plus_Key});
  } else if (strcmp(name, "c_down") == 0) {
    send_command({Key_On, C_Minus_Key});
  } else if (strcmp(name, "c_up") == 0) {
    send_command({Key_On, C_Plus_Key});
  } else if (strcmp(name, "tune") == 0) {
    send_command({Key_On, Tune_Key});
  } else if (strcmp(name, "input") == 0) {
    send_command({Key_On, In_Key});
  } else if (strcmp(name, "band_down") == 0) {
    send_command({Key_On, Band_Minus_Key});
  } else if (strcmp(name, "band_up") == 0) {
    send_command({Key_On, Band_Plus_Key});
  } else if (strcmp(name, "ant") == 0) {
    send_command({Key_On, Ant_Key});
  } else if (strcmp(name, "left") == 0) {
    send_command({Key_On, Left_Key});
  } else if (strcmp(name, "right") == 0) {
    send_command({Key_On, Right_Key});
  } else if (strcmp(name, "cat") == 0) {
    send_command({Key_On, Cat_Key});
  } else if (strcmp(name, "set") == 0) {
    send_command({Key_On, Set_Key});
  } else if (strcmp(name, "off") == 0) {
    send_command({Key_On, Off_Key});
  } else if (strcmp(name, "on") == 0) {
    send_command({Rcu_On});
  } else if (strcmp(name, "power") == 0) {
    send_command({Key_On, Power_Key});
  } else if (strcmp(name, "display") == 0) {
    send_command({Key_On, Display_Key});
  } else if (strcmp(name, "operate") == 0) {
    send_command({Key_On, Operate_Key});
  } else {
    return false;
  }

  return true;
}

/* Rather than have a separate function for every button, we check who the calling object it and act on it
L_Plus_Key=0x30,L_Minus_Key=0x31,C_Minus_Key=0x32,C_Plus_Key=0x33,Tune_Key=0x34,In_Key=0x28,Band_Minus_Key=0x29,Band_Plus_Key=0x2A,
Ant_Key=0x2B,Cat_Key=0x2C,Left_Key=0x2D,Right_Key=0x2E,Set_Key=0x2F,Off_Key=0x18,Power_Key=0x1A,Display_Key=0x1B,Operate_Key=0x1C
*/

void button_pressed(lv_event_t * e)
{
  hold = false;
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_current_target_obj(e);
  if(code == LV_EVENT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
    if (obj == ui_buttonLowerL)
      send_command({Key_On,L_Minus_Key});
    else if (obj == ui_buttonHigherL)
      send_command({Key_On,L_Plus_Key});
    else if (obj == ui_buttonLowerC)
      send_command({Key_On,C_Minus_Key});
    else if (obj == ui_buttonHigherC)
      send_command({Key_On,C_Plus_Key});
    else if (obj == ui_buttonTune)
      send_command({Key_On,Tune_Key});
    else if (obj == ui_buttonInput)
      send_command({Key_On,In_Key});
    if (obj == ui_buttonBandDown)
      send_command({Key_On,Band_Minus_Key});
    else if (obj == ui_buttonBandUp)
      send_command({Key_On,Band_Plus_Key});
    if (obj == ui_buttonAnt)
      send_command({Key_On,Ant_Key});
    if (obj == ui_buttonLeftUp)
      send_command({Key_On,Left_Key});
    else if (obj == ui_buttonRightDown)
      send_command({Key_On,Right_Key});
    else if (obj == ui_buttonCat)
      send_command({Key_On,Cat_Key});
    else if (obj == ui_buttonSet)
      send_command({Key_On,Set_Key});
    else if (obj == ui_buttonOff)
      send_command({Key_On,Off_Key});
    else if (obj == ui_buttonOn) 
      send_command({Rcu_On});
    else if (obj == ui_buttonPower)
      send_command({Key_On,Power_Key});
    else if (obj == ui_buttonDisplay)
      send_command({Key_On,Display_Key});
    else if (obj == ui_buttonOperate)
      send_command({Key_On,Operate_Key});
  }
}
