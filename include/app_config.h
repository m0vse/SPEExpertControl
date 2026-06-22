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

struct AppConfig {
    char wifi_ssid[APP_CONFIG_WIFI_SSID_LEN + 1] = "";
    char wifi_password[APP_CONFIG_WIFI_PASSWORD_LEN + 1] = "";
    bool display_flipped = false;
    bool amp_serial_usb = false;
};

bool app_config_load(void);
bool app_config_save(void);
AppConfig app_config_snapshot(void);
bool app_config_display_flipped(void);
bool app_config_set_display_flipped(bool flipped);
bool app_config_amp_serial_usb(void);
bool app_config_set_amp_serial_usb(bool enabled);
bool app_config_set_wifi_credentials(const char *ssid, const char *password);
bool app_config_clear_wifi_credentials(void);
