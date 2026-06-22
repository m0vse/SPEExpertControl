/*
 * Main application entry point, FreeRTOS task setup, amplifier packet handling, and LCD UI state updates.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include <Arduino.h>
#include <platform/mbed_stats.h>
#include <rtos.h>

#include "Arduino_GigaDisplayTouch.h"
#include "amp_control.h"
#include "app_config.h"
#include "app_status.h"
#include "console/serial_console.h"
#include "display/lvgl_giga_display.h"
#include "display/lvgl_giga_touch.h"
#include "lvgl.h"
#include <ui.h>
#include "amp_dtr.h"
#include "models/spe_expert1k/expertpackets.h"
#include "models/spe_expert1k/lcd_renderer.h"
#include "models/spe_expert1k/menuitems.h"
#include "models/spe_expert1k/protocol.h"
#include "models/spe_expert1k/runtime.h"
#include "models/spe_expert1k/serial_link.h"
#include "serial/transport_stats.h"
#include "ui/menu_control.h"

#if SPE_ENABLE_WEB_SERVER
#include "network/control_server.h"
#endif

#if SPE_ENABLE_WIFI_SETUP || SPE_ENABLE_WEB_SERVER
#include "network/wifi_lock.h"
#endif

#if SPE_ENABLE_WIFI_SETUP
#include "ui/wifi_setup.h"
#endif

#if SPE_ENABLE_WIFI_SETUP || SPE_ENABLE_WEB_SERVER
#include <WiFi.h>
#endif

#include <string.h>
#include <stdlib.h>

using namespace std::chrono_literals;

#ifndef SPE_BRINGUP_LEVEL
#define SPE_BRINGUP_LEVEL 5
#endif

#ifndef SPE_VERBOSE_PACKET_LOG
#define SPE_VERBOSE_PACKET_LOG 0
#endif

uint8_t progress = 0;

#define COUNT_OF(a) ((int)(sizeof(a) / sizeof((a)[0])))

void process_packet(const Expert_Packet &packet_in, uint8_t len_in);
void ui_task(void);
void serial_task(void);
void console_task(void);
void wifi_task(void);
void web_task(void);

const char* baud_rate[] = {"1200 Baud","2400 Baud","4800 Baud","9600 Baud"};

const char* setup_options[] = {"ANTENNA","CAT","MANUAL TUNE","BACKLIGHT","CONTEST","BEEP","START","TEMP","QUIT"};

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

// Amplifier settings
const unsigned long interval = 1000;
bool touch_ready = false;
int transformed_touch_devices = 0;
static SpeExpert1kRuntime amp_runtime;
static rtos::Mutex lvgl_mutex;
static rtos::Mutex debug_serial_mutex;
static rtos::Thread ui_thread(osPriorityNormal, 8192, nullptr, "ui");
static rtos::Thread serial_thread(osPriorityHigh, 8192, nullptr, "amp");
static rtos::Thread console_thread(osPriorityBelowNormal, 4096, nullptr, "console");
static rtos::Thread wifi_thread(osPriorityNormal, 8192, nullptr, "wifi");
static rtos::Thread web_thread(osPriorityBelowNormal, 8192, nullptr, "web");

class LvglLock {
public:
  LvglLock() { lvgl_mutex.lock(); }
  ~LvglLock() { lvgl_mutex.unlock(); }
};

class DebugSerialLock {
public:
  DebugSerialLock() { debug_serial_mutex.lock(); }
  ~DebugSerialLock() { debug_serial_mutex.unlock(); }
};

static void process_next_queued_amp_packet()
{
  SpeExpert1kQueuedPacket queued;
  if (spe_expert1k_dequeue_packet(queued)) {
    process_packet(queued.packet, queued.len);
  }
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

static void print_console_help()
{
  DebugSerialLock debug_lock;
  Serial.println(F("Commands:"));
  Serial.println(F("  help        Show this help"));
  Serial.println(F("  status      Controller summary"));
  Serial.println(F("  wifi        WiFi status, IP and firmware"));
  Serial.println(F("  web         HTTP server counters"));
  Serial.println(F("  serial      Amplifier serial health counters"));
  Serial.println(F("  amp         Last amplifier status packet"));
  Serial.println(F("  model       Show or set amplifier model"));
  Serial.println(F("  scan        Blocking WiFi scan to serial"));
  Serial.println(F("  rcu         Send RCU_ON to the amplifier"));
  Serial.println(F("  dtr         Show amplifier DTR output state"));
  Serial.println(F("  amp-serial  Show or set amplifier serial transport"));
  Serial.println(F("  amp-baud    Show or set amplifier serial baud rate"));
  Serial.println(F("  exit        Return USB serial to amplifier comms"));
  Serial.println(F("  setup       Open controller setup popup on the display"));
  Serial.println(F("  wifi-saved  Show saved WiFi credential state"));
  Serial.println(F("  wifi-clear  Clear saved WiFi credentials"));
  Serial.println(F("  stats       CPU, heap and RTOS thread stats"));
  Serial.println(F("  reboot      Reboot the controller"));
  Serial.println(F("  dfu         Reboot into the bootloader"));
  Serial.println(F("  poll on     Enable periodic status polling"));
  Serial.println(F("  poll off    Disable periodic status polling"));
}

static void print_wifi_status()
{
#if SPE_ENABLE_WIFI_SETUP || SPE_ENABLE_WEB_SERVER
  int status = WL_IDLE_STATUS;
  String firmware;
  String ssid;
  IPAddress ip;
  long rssi = 0;
  {
    WifiStackLock wifi_lock;
    status = WiFi.status();
    firmware = WiFi.firmwareVersion();
    if (status == WL_CONNECTED) {
      ssid = WiFi.SSID();
      ip = WiFi.localIP();
      rssi = WiFi.RSSI();
    }
  }
  const bool setup_connected =
#if SPE_ENABLE_WIFI_SETUP
    wifi_setup_is_connected();
#else
    false;
#endif
  const bool has_saved_credentials =
#if SPE_ENABLE_WIFI_SETUP
    wifi_setup_has_saved_credentials();
#else
    false;
#endif
  DebugSerialLock debug_lock;
  Serial.print(F("WiFi status="));
  Serial.print(status);
  Serial.print(F(" ("));
  Serial.print(wifi_status_name(status));
  Serial.println(F(")"));
  Serial.print(F("WiFi firmware="));
  Serial.println(firmware);
  Serial.print(F("WiFi setup connected="));
#if SPE_ENABLE_WIFI_SETUP
  Serial.println(setup_connected ? F("yes") : F("no"));
  Serial.print(F("WiFi saved credentials="));
  Serial.println(has_saved_credentials ? F("yes") : F("no"));
#else
  Serial.println(F("n/a"));
#endif
  if (status == WL_CONNECTED) {
    Serial.print(F("SSID="));
    Serial.println(ssid);
    Serial.print(F("IP="));
    Serial.println(ip);
    Serial.print(F("RSSI="));
    Serial.print(rssi);
    Serial.println(F(" dBm"));
  }
#else
  DebugSerialLock debug_lock;
  Serial.println(F("WiFi disabled at build time"));
#endif
}

static void print_web_status()
{
#if SPE_ENABLE_WEB_SERVER
  DebugSerialLock debug_lock;
  control_server_print_stats(Serial);
#else
  DebugSerialLock debug_lock;
  Serial.println(F("Web server disabled at build time"));
#endif
}

static void print_serial_status()
{
  const SerialTransportStats snapshot = serial_transport_stats_snapshot();

  DebugSerialLock debug_lock;
  Serial.println(F("Amplifier serial stats:"));
  Serial.print(F("  transport="));
  Serial.println(app_config_amp_serial_port_name(spe_expert1k_amp_serial_port()));
  Serial.print(F("  baud="));
  Serial.println(app_config_amp_baud());
  Serial.print(F("  usb_console_active="));
  Serial.println(spe_expert1k_usb_console_active() ? F("yes") : F("no"));
  Serial.print(F("  rx_bytes="));
  Serial.println(snapshot.rx_bytes);
  Serial.print(F("  valid_packets="));
  Serial.println(snapshot.valid_packets);
  Serial.print(F("  invalid_checksums="));
  Serial.println(snapshot.invalid_checksums);
  Serial.print(F("  max_available="));
  Serial.println(snapshot.max_available);
  Serial.print(F("  max_task_gap_ms="));
  Serial.println(snapshot.max_task_gap_ms);
  Serial.print(F("  max_rx_byte_gap_us="));
  Serial.println(snapshot.max_rx_byte_gap_us);
  Serial.print(F("  max_drain_burst="));
  Serial.println(snapshot.max_drain_burst);
  Serial.print(F("  max_queue_depth="));
  Serial.println(snapshot.max_queue_depth);
  Serial.print(F("  queued_packets_dropped="));
  Serial.println(snapshot.queued_packets_dropped);
  Serial.print(F("  max_command_queue_depth="));
  Serial.println(snapshot.max_command_queue_depth);
  Serial.print(F("  queued_commands_dropped="));
  Serial.println(snapshot.queued_commands_dropped);
  Serial.print(F("  commands_sent="));
  Serial.println(snapshot.commands_sent);
  Serial.print(F("  last_checksum_error_ms="));
  Serial.println(snapshot.last_checksum_error_ms);
  Serial.print(F("  checksum_sync_resyncs="));
  Serial.println(snapshot.checksum_sync_resyncs);
  Serial.print(F("  last_bad_available="));
  Serial.println(snapshot.last_bad_available);
  Serial.print(F("  last_bad_packet_len="));
  Serial.println(snapshot.last_bad_packet_len);
  Serial.print(F("  last_bad_checksum_expected=0x"));
  Serial.println(snapshot.last_bad_checksum_expected, HEX);
  Serial.print(F("  last_bad_checksum_received=0x"));
  Serial.println(snapshot.last_bad_checksum_received, HEX);
}

static void print_amp_status()
{
  const AppStatusSnapshot snapshot = app_status_snapshot();
  const AmpStatusSnapshot &status = snapshot.amp;

  DebugSerialLock debug_lock;
  Serial.print(F("Amp status valid="));
  Serial.println(snapshot.valid ? F("yes") : F("no"));
  Serial.print(F("model="));
  Serial.println(amp_model_label(amp_runtime.model_id()));
  Serial.print(F("screen="));
  Serial.print(status.screen_id);
  Serial.print(F(" ("));
  Serial.print(status.screen_name);
  Serial.println(F(")"));
  Serial.print(F("last_rcu_ms="));
  Serial.println(amp_runtime.last_activity_ms());
  Serial.print(F("dtr_pin=D"));
  Serial.print(amp_dtr_pin());
  Serial.print(F(" asserted="));
  Serial.print(amp_dtr_is_asserted() ? F("yes") : F("no"));
  Serial.print(F(" gpio_level="));
  Serial.println(amp_dtr_gpio_level() == HIGH ? F("HIGH") : F("LOW"));

  if (!snapshot.valid) {
    Serial.println(F("No 30-byte amplifier status packet received yet"));
    return;
  }

  Serial.print(F("band="));
  Serial.print(status.band);
  Serial.print(F(" input="));
  Serial.print(status.input);
  Serial.print(F(" antenna="));
  Serial.print(status.antenna);
  Serial.print(F(" cat="));
  Serial.print(status.cat);
  Serial.print(F(" out="));
  Serial.println(status.out);

  Serial.print(F("power="));
  Serial.print(status.power);
  Serial.print(F("W rev="));
  Serial.print(status.reverse);
  Serial.print(F("W swr="));
  Serial.print(status.swr);
  Serial.print(F(" temp="));
  Serial.print(status.temp);
  Serial.print(F(" voltage="));
  Serial.print(status.voltage);
  Serial.print(F(" current="));
  Serial.println(status.current);
}

static void print_controller_status()
{
  const AppStatusSnapshot snapshot = app_status_snapshot();
  const AmpStatusSnapshot &status = snapshot.amp;

  DebugSerialLock debug_lock;
  Serial.print(F("Controller ms="));
  Serial.print(millis());
  Serial.print(F(" model="));
  Serial.print(amp_model_key(amp_runtime.model_id()));
  Serial.print(F(" progress="));
  Serial.print(progress);
  Serial.print(F(" touch="));
  Serial.print(touch_ready ? F("ok") : F("failed"));
  Serial.print(F(" indev="));
  Serial.print(transformed_touch_devices);
  Serial.print(F(" screen="));
  Serial.print(status.screen_id);
  Serial.print(F(" ("));
  Serial.print(status.screen_name);
  Serial.println(F(")"));
#if SPE_BRINGUP_LEVEL >= 5
  Serial.print(F("serial1_available="));
  Serial.println(spe_expert1k_serial_available());
#endif
}

static void print_wifi_scan()
{
#if SPE_ENABLE_WIFI_SETUP || SPE_ENABLE_WEB_SERVER
  {
    DebugSerialLock debug_lock;
    Serial.println(F("Starting blocking WiFi scan..."));
  }
  int networks = 0;
  {
    WifiStackLock wifi_lock;
    networks = WiFi.scanNetworks();
  }
  {
    DebugSerialLock debug_lock;
    Serial.print(F("WiFi networks found="));
    Serial.println(networks);
    for (int i = 0; i < networks; ++i) {
      String ssid;
      long rssi = 0;
      uint8_t encryption = 0;
      {
        WifiStackLock wifi_lock;
        ssid = WiFi.SSID(i);
        rssi = WiFi.RSSI(i);
        encryption = WiFi.encryptionType(i);
      }
      Serial.print(i);
      Serial.print(F(": "));
      Serial.print(ssid);
      Serial.print(F(" RSSI="));
      Serial.print(rssi);
      Serial.print(F(" enc="));
      Serial.println(encryption);
    }
  }
#else
  DebugSerialLock debug_lock;
  Serial.println(F("WiFi disabled at build time"));
#endif
}

static void print_runtime_stats()
{
  mbed_stats_heap_t heap;
  mbed_stats_stack_t stack;
  mbed_stats_cpu_t cpu;
  mbed_stats_thread_t threads[12];

  mbed_stats_heap_get(&heap);
  mbed_stats_stack_get(&stack);
  mbed_stats_cpu_get(&cpu);
  const size_t thread_count = mbed_stats_thread_get_each(threads, COUNT_OF(threads));

  DebugSerialLock debug_lock;
  Serial.println(F("Runtime stats:"));
  Serial.print(F("  uptime_ms="));
  Serial.println(static_cast<unsigned long>(cpu.uptime / 1000ULL));
  Serial.print(F("  idle_ms="));
  Serial.println(static_cast<unsigned long>(cpu.idle_time / 1000ULL));
  Serial.print(F("  sleep_ms="));
  Serial.println(static_cast<unsigned long>(cpu.sleep_time / 1000ULL));
  Serial.print(F("  deep_sleep_ms="));
  Serial.println(static_cast<unsigned long>(cpu.deep_sleep_time / 1000ULL));

  Serial.print(F("  heap_current="));
  Serial.print(heap.current_size);
  Serial.print(F(" max="));
  Serial.print(heap.max_size);
  Serial.print(F(" reserved="));
  Serial.print(heap.reserved_size);
  Serial.print(F(" allocs="));
  Serial.print(heap.alloc_cnt);
  Serial.print(F(" fails="));
  Serial.println(heap.alloc_fail_cnt);

  Serial.print(F("  stack_total_used_max="));
  Serial.print(stack.max_size);
  Serial.print(F(" reserved="));
  Serial.print(stack.reserved_size);
  Serial.print(F(" count="));
  Serial.println(stack.stack_cnt);

  Serial.println(F("  threads:"));
  for (size_t i = 0; i < thread_count; ++i) {
    Serial.print(F("    id=0x"));
    Serial.print(threads[i].id, HEX);
    Serial.print(F(" name="));
    Serial.print(threads[i].name ? threads[i].name : "(null)");
    Serial.print(F(" state="));
    Serial.print(threads[i].state);
    Serial.print(F(" prio="));
    Serial.print(threads[i].priority);
    Serial.print(F(" stack="));
    Serial.print(threads[i].stack_size);
    Serial.print(F(" free="));
    Serial.println(threads[i].stack_space);
  }
  if (thread_count == COUNT_OF(threads)) {
    Serial.println(F("  thread list may be truncated"));
  }
}

static void reboot_controller()
{
  {
    DebugSerialLock debug_lock;
    Serial.println(F("Rebooting controller..."));
    Serial.flush();
  }
  delay(100);
  NVIC_SystemReset();
}

static void enter_dfu_bootloader()
{
  {
    DebugSerialLock debug_lock;
    Serial.println(F("Rebooting into DFU bootloader..."));
    Serial.flush();
  }
  delay(100);
  _ontouch1200bps_();
}

static void print_amp_model_config()
{
  uint8_t count = 0;
  const AmpModelInfo *models = amp_model_catalog(count);

  DebugSerialLock debug_lock;
  Serial.print(F("Active amplifier model: "));
  Serial.println(amp_model_label(amp_runtime.model_id()));
  Serial.print(F("Saved amplifier model: "));
  Serial.println(amp_model_label(app_config_amp_model()));
  Serial.println(F("Known models:"));
  for (uint8_t i = 0; i < count; ++i) {
    Serial.print(F("  "));
    Serial.print(models[i].key);
    Serial.print(F(" - "));
    Serial.print(models[i].label);
    Serial.print(F(" ["));
    Serial.print(models[i].available ? F("available") : F("not implemented"));
    Serial.println(F("]"));
  }
}

static void set_amp_model_config(AmpModelId model)
{
  if (!amp_model_available(model)) {
    DebugSerialLock debug_lock;
    Serial.print(F("Amplifier model is not implemented yet: "));
    Serial.println(amp_model_label(model));
    return;
  }

  const AmpModelId saved = app_config_amp_model();
  if (saved == model) {
    DebugSerialLock debug_lock;
    Serial.print(F("Amplifier model already saved as "));
    Serial.println(amp_model_label(model));
    return;
  }

  if (!app_config_set_amp_model(model)) {
    DebugSerialLock debug_lock;
    Serial.println(F("Failed to save amplifier model"));
    return;
  }

  {
    DebugSerialLock debug_lock;
    Serial.print(F("Saved amplifier model as "));
    Serial.print(amp_model_label(model));
    Serial.println(F("; rebooting to apply"));
    Serial.flush();
  }
  delay(100);
  NVIC_SystemReset();
}

static void print_amp_serial_config()
{
  DebugSerialLock debug_lock;
  Serial.print(F("Active amplifier serial transport: "));
  Serial.println(app_config_amp_serial_port_name(spe_expert1k_amp_serial_port()));
  Serial.print(F("Saved amplifier serial transport: "));
  Serial.println(app_config_amp_serial_port_name(app_config_amp_serial_port()));
  Serial.print(F("Saved amplifier baud rate: "));
  Serial.println(app_config_amp_baud());
  Serial.println(F("Use 'amp-serial uart1', 'uart2', 'uart3', 'uart4', or 'usb' to change it."));
  Serial.println(F("Use 'amp-baud 9600' through 'amp-baud 115200' to change baud."));
}

static void set_amp_serial_config(AppAmpSerialPort port)
{
  const AppAmpSerialPort saved = app_config_amp_serial_port();
  if (saved == port) {
    DebugSerialLock debug_lock;
    Serial.print(F("Amplifier serial transport already saved as "));
    Serial.println(app_config_amp_serial_port_name(port));
    return;
  }

  if (!app_config_set_amp_serial_port(port)) {
    DebugSerialLock debug_lock;
    Serial.println(F("Failed to save amplifier serial transport setting"));
    return;
  }

  {
    DebugSerialLock debug_lock;
    Serial.print(F("Saved amplifier serial transport as "));
    Serial.print(app_config_amp_serial_port_name(port));
    Serial.println(F("; rebooting to apply"));
    Serial.flush();
  }
  delay(100);
  NVIC_SystemReset();
}

static void print_amp_baud_config()
{
  DebugSerialLock debug_lock;
  Serial.print(F("Saved amplifier baud rate: "));
  Serial.println(app_config_amp_baud());
  Serial.println(F("Supported: 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200"));
}

static void set_amp_baud_config(uint32_t baud)
{
  if (!app_config_is_valid_amp_baud(baud)) {
    DebugSerialLock debug_lock;
    Serial.println(F("Unsupported amplifier baud rate"));
    Serial.println(F("Supported: 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200"));
    return;
  }

  const uint32_t saved = app_config_amp_baud();
  if (saved == baud) {
    DebugSerialLock debug_lock;
    Serial.print(F("Amplifier baud rate already saved as "));
    Serial.println(baud);
    return;
  }

  if (!app_config_set_amp_baud(baud)) {
    DebugSerialLock debug_lock;
    Serial.println(F("Failed to save amplifier baud rate"));
    return;
  }

  {
    DebugSerialLock debug_lock;
    Serial.print(F("Saved amplifier baud rate as "));
    Serial.print(baud);
    Serial.println(F("; rebooting to apply"));
    Serial.flush();
  }
  delay(100);
  NVIC_SystemReset();
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
  } else if (strcmp(line, "web") == 0) {
    print_web_status();
  } else if (strcmp(line, "serial") == 0 || strcmp(line, "ser") == 0) {
    print_serial_status();
  } else if (strcmp(line, "amp") == 0) {
    print_amp_status();
  } else if (strcmp(line, "model") == 0 || strcmp(line, "amp-model") == 0 || strcmp(line, "ampmodel") == 0) {
    print_amp_model_config();
  } else if (strncmp(line, "model ", 6) == 0) {
    AmpModelId model = AmpModelId::Unknown;
    if (amp_model_parse(line + 6, model)) {
      set_amp_model_config(model);
    } else {
      DebugSerialLock debug_lock;
      Serial.println(F("Unknown amplifier model. Run 'model' to list known models."));
    }
  } else if (strncmp(line, "amp-model ", 10) == 0) {
    AmpModelId model = AmpModelId::Unknown;
    if (amp_model_parse(line + 10, model)) {
      set_amp_model_config(model);
    } else {
      DebugSerialLock debug_lock;
      Serial.println(F("Unknown amplifier model. Run 'model' to list known models."));
    }
  } else if (strcmp(line, "scan") == 0) {
    print_wifi_scan();
  } else if (strcmp(line, "stats") == 0 || strcmp(line, "mem") == 0) {
    print_runtime_stats();
  } else if (strcmp(line, "reboot") == 0 || strcmp(line, "reset") == 0) {
    reboot_controller();
  } else if (strcmp(line, "dfu") == 0 || strcmp(line, "bootloader") == 0) {
    enter_dfu_bootloader();
  } else if (strcmp(line, "rcu") == 0) {
    amp_control_power_on();
  } else if (strcmp(line, "dtr") == 0) {
    DebugSerialLock debug_lock;
    Serial.print(F("Amp DTR pin=D"));
    Serial.print(amp_dtr_pin());
    Serial.print(F(" asserted="));
    Serial.print(amp_dtr_is_asserted() ? F("yes") : F("no"));
    Serial.print(F(" gpio_level="));
    Serial.println(amp_dtr_gpio_level() == HIGH ? F("HIGH") : F("LOW"));
  } else if (strcmp(line, "amp-serial") == 0 || strcmp(line, "ampserial") == 0) {
    print_amp_serial_config();
  } else if (strncmp(line, "amp-serial ", 11) == 0) {
    AppAmpSerialPort port = AppAmpSerialPort::Uart1;
    if (app_config_parse_amp_serial_port(line + 11, port)) {
      set_amp_serial_config(port);
    } else {
      DebugSerialLock debug_lock;
      Serial.println(F("Unsupported amplifier serial transport. Use uart1, uart2, uart3, uart4, or usb."));
    }
  } else if (strncmp(line, "ampserial ", 10) == 0) {
    AppAmpSerialPort port = AppAmpSerialPort::Uart1;
    if (app_config_parse_amp_serial_port(line + 10, port)) {
      set_amp_serial_config(port);
    } else {
      DebugSerialLock debug_lock;
      Serial.println(F("Unsupported amplifier serial transport. Use uart1, uart2, uart3, uart4, or usb."));
    }
  } else if (strcmp(line, "amp-baud") == 0 || strcmp(line, "ampbaud") == 0) {
    print_amp_baud_config();
  } else if (strncmp(line, "amp-baud ", 9) == 0) {
    set_amp_baud_config(strtoul(line + 9, nullptr, 10));
  } else if (strncmp(line, "ampbaud ", 8) == 0) {
    set_amp_baud_config(strtoul(line + 8, nullptr, 10));
  } else if (strcmp(line, "exit") == 0 || strcmp(line, "passthrough") == 0) {
    if (spe_expert1k_amp_uses_usb_serial()) {
      DebugSerialLock debug_lock;
      Serial.println(F("Returning USB serial to amplifier comms"));
      Serial.flush();
      serial_console_reset_line();
      spe_expert1k_usb_console_release();
    } else {
      DebugSerialLock debug_lock;
      Serial.println(F("Amplifier comms are not using USB Serial; console remains active"));
    }
  } else if (strcmp(line, "setup") == 0 || strcmp(line, "wifi-popup") == 0) {
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
    serial_console_set_poll_enabled(true);
    DebugSerialLock debug_lock;
    Serial.println(F("Periodic status polling enabled"));
  } else if (strcmp(line, "poll off") == 0) {
    serial_console_set_poll_enabled(false);
    DebugSerialLock debug_lock;
    Serial.println(F("Periodic status polling disabled"));
  } else {
    DebugSerialLock debug_lock;
    Serial.print(F("Unknown command: "));
    Serial.println(line);
    Serial.println(F("Type help for commands"));
  }
}

static void print_console_poll_status(unsigned long now)
{
#if SPE_BRINGUP_LEVEL >= 5
  int serial1_available = 0;
  {
    serial1_available = spe_expert1k_serial_available();
  }
#endif

  DebugSerialLock debug_lock;
  Serial.print(F("Status: ms="));
  Serial.print(now);
  Serial.print(F(" screen="));
  Serial.print(static_cast<int>(amp_runtime.screen()));
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
  amp_control_bind_runtime(&amp_runtime);

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
  app_config_load();
  giga_lvgl_display_set_flipped(app_config_display_flipped());
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
  spe_expert1k_serial_begin(); // Amp connection
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

  amp_runtime.mark_activity(millis());
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
  console_thread.start(console_task);
#endif

#if SPE_ENABLE_WIFI_SETUP && SPE_BRINGUP_LEVEL >= 4
  wifi_thread.start(wifi_task);
#endif

#if SPE_ENABLE_WEB_SERVER && SPE_BRINGUP_LEVEL >= 5
  web_thread.start(web_task);
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

    process_next_queued_amp_packet();
    rtos::ThisThread::sleep_for(5ms);
  }
}

void serial_task()
{
  uint32_t last_run_ms = millis();
  bool last_usb_console_active = false;
  while (true) {
  const bool usb_console_active = spe_expert1k_usb_console_active();
  if (usb_console_active) {
    if (!last_usb_console_active) {
      serial_console_reset_line();
      DebugSerialLock debug_lock;
      Serial.println(F("Console ready. Type 'help' for commands."));
    }
    last_usb_console_active = true;
    serial_console_service(handle_console_command, print_console_poll_status);
    rtos::ThisThread::sleep_for(10ms);
    continue;
  }
  last_usb_console_active = false;

  const uint32_t now_ms = millis();
  serial_transport_note_task_gap(now_ms - last_run_ms);
  last_run_ms = now_ms;
  uint32_t drained_bytes = 0;
  bool completed_packet = false;

  // Check for serial data
  while (true) {
    SpeExpert1kReadResult read_result;
    if (!spe_expert1k_serial_read(read_result)) {
      break;
    }
    ++drained_bytes;

    switch (read_result.result) {
      case ExpertPacketParser::Result::PacketReady:
        serial_transport_note_valid_packet();
        completed_packet = true;
        if (read_result.len == 30) {
          amp_runtime.mark_activity(millis());
          spe_expert1k_queue_packet(read_result.packet, read_result.len);
        } else {
          process_packet(read_result.packet, read_result.len);
        }
        break;
      case ExpertPacketParser::Result::InvalidChecksum:
      {
        serial_transport_note_invalid_checksum(
          read_result.invalid_len,
          read_result.invalid_expected_checksum,
          read_result.invalid_received_checksum,
          read_result.last_available);
        break;
      }
      case ExpertPacketParser::Result::None:
        break;
    }
  }  
  serial_transport_note_drain_burst(drained_bytes);

// Keep the amplifier remote-control stream alive if no status packets arrive.
  unsigned long now = millis();
  if (amp_control_remote_updates_enabled() && amp_runtime.should_send_keepalive(now, interval))
  {
    //Serial1.end();      // close serial port
    //delay(100);        //wait 100 millis
    //Serial1.begin(9600);
    amp_control_power_on();
    completed_packet = true;
    amp_runtime.note_keepalive(now);
    if (progress < 100) {
      progress++;
    }
    {
      LvglLock lock;
      lv_bar_set_value(ui_startupBar, progress, LV_ANIM_OFF);
    }
  }

    if (completed_packet) {
      amp_control_process_next_queued_command();
    }
    rtos::ThisThread::sleep_for(1ms);
  }
}

void console_task()
{
  while (true) {
    if (!spe_expert1k_amp_uses_usb_serial()) {
      serial_console_service(handle_console_command, print_console_poll_status);
    }
    rtos::ThisThread::sleep_for(10ms);
  }
}

void wifi_task()
{
  rtos::ThisThread::sleep_for(2s);
  while (true) {
#if SPE_ENABLE_WIFI_SETUP && SPE_BRINGUP_LEVEL >= 4
    wifi_setup_connection_service();
#endif
    rtos::ThisThread::sleep_for(100ms);
  }
}

void web_task()
{
  rtos::ThisThread::sleep_for(3s);
  while (true) {
#if SPE_ENABLE_WEB_SERVER && SPE_BRINGUP_LEVEL >= 5
    #if SPE_ENABLE_WIFI_SETUP
    if (wifi_setup_is_connected()) {
      control_server_service();
    } else {
      rtos::ThisThread::sleep_for(250ms);
    }
    #else
    control_server_service();
    #endif
#endif
    rtos::ThisThread::sleep_for(5ms);
  }
}

static SpeExpert1kRuntimeBindings spe_runtime_bindings()
{
  return {
    &setup_options_ctrl,
    &setup_ant_ctrl,
    &setup_cat_ctrl,
    &setup_yaesu_ctrl,
    &setup_icom_ctrl,
    &setup_tentec_ctrl,
    &setup_baudrate_ctrl,
    setup_ant_items,
    COUNT_OF(setup_ant_items)
  };
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
    const unsigned long now = millis();
    LvglLock lock;

    // Any valid status packet means the amp serial link is alive.
    if (amp_runtime.screen() == BootMessage) {
#if SPE_ENABLE_WIFI_SETUP
        wifi_setup_set_visible(false);
#endif
        lv_disp_load_scr(ui_mainScreen);
    }

    amp_runtime.process_status_packet(packet_in, spe_runtime_bindings(), now);

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
  }
}

