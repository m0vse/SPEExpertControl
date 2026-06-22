/*
 * Application status export API used to expose amplifier state as JSON.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <Arduino.h>
#include <Print.h>
#include "models/amplifier_model.h"

static constexpr uint8_t APP_MODEL_DATA_MAX = 16;

struct AppAmpTextStorage {
    char screen_name[32] = "unknown";
    char input[8] = "?";
    char band[12] = "?";
    char antenna[8] = "?";
    char cat[8] = "?";
    char out[8] = "?";
    char lcd_title[41] = "";
    char lcd_body[336] = "";
    char lcd_cells[641] = "";
    char lcd_attrs[81] = "";
    char lcd_footer[41] = "";
    char lcd_hint[100] = "";
};

struct AppModelData {
    AmpModelId model = AmpModelId::Unknown;
    uint8_t screen_id = 0;
    const char *screen_name = "unknown";
    uint8_t bytes[APP_MODEL_DATA_MAX] = {};
    uint8_t size = 0;
};

struct AppStatusSnapshot {
    bool valid = false;
    AmpStatusSnapshot amp{};
    AppModelData model_data{};
    bool transient_valid = false;
    AmpStatusSnapshot transient_amp{};
    AppModelData transient_model_data{};
    unsigned long transient_until = 0;
    uint32_t sequence = 0;
    AppAmpTextStorage amp_text{};
    AppAmpTextStorage transient_amp_text{};
    char model_screen_name[32] = "unknown";
    char transient_model_screen_name[32] = "unknown";
};

/**
 * Publish the latest decoded amplifier/UI state.
 */
void app_status_publish(const AppStatusSnapshot &snapshot);

/**
 * Return a thread-safe copy of the latest published state.
 */
AppStatusSnapshot app_status_snapshot(void);

/**
 * Write the latest amplifier/UI state as a JSON object to any Arduino Print stream.
 *
 * The caller owns the surrounding JSON context and stream lifetime.
 */
void app_status_print_json(Print &out);

/**
 * Return the monotonically increasing sequence number for published amp status.
 */
uint32_t app_status_sequence(void);
