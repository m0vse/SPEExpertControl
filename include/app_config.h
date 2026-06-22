/*
 * Persistent controller configuration stored on the Giga QSPI filesystem.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <Arduino.h>

static const size_t APP_CONFIG_WIFI_SSID_LEN = 32;
static const size_t APP_CONFIG_WIFI_PASSWORD_LEN = 64;
static const uint32_t APP_CONFIG_DEFAULT_AMP_BAUD = 9600;

enum class AppAmpSerialPort : uint8_t {
    Uart1 = 0,
    Uart2,
    Uart3,
    Uart4,
    Usb,
};

struct AppConfig {
    char wifi_ssid[APP_CONFIG_WIFI_SSID_LEN + 1] = "";
    char wifi_password[APP_CONFIG_WIFI_PASSWORD_LEN + 1] = "";
    bool display_flipped = false;
    AppAmpSerialPort amp_serial_port = AppAmpSerialPort::Uart1;
    uint32_t amp_baud = APP_CONFIG_DEFAULT_AMP_BAUD;
};

bool app_config_load(void);
bool app_config_save(void);
AppConfig app_config_snapshot(void);
bool app_config_display_flipped(void);
bool app_config_set_display_flipped(bool flipped);
AppAmpSerialPort app_config_amp_serial_port(void);
const char *app_config_amp_serial_port_name(AppAmpSerialPort port);
bool app_config_parse_amp_serial_port(const char *value, AppAmpSerialPort &port);
bool app_config_set_amp_serial_port(AppAmpSerialPort port);
uint32_t app_config_amp_baud(void);
bool app_config_is_valid_amp_baud(uint32_t baud);
bool app_config_set_amp_baud(uint32_t baud);
bool app_config_set_wifi_credentials(const char *ssid, const char *password);
bool app_config_clear_wifi_credentials(void);
