/*
 * Model-neutral amplifier status types shared by UI, web, and future protocol implementations.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <stdint.h>

enum class AmpModelId : uint8_t {
    Unknown = 0,
    SpeExpert1k,
    SpeModern,
    SpeExpert13k,
    SpeExpert15k,
    SpeExpert2k
};

struct AmpModelInfo {
    AmpModelId id = AmpModelId::Unknown;
    const char *key = "unknown";
    const char *label = "Unknown";
    bool available = false;
};

const AmpModelInfo *amp_model_catalog(uint8_t &count);
const AmpModelInfo *amp_model_info(AmpModelId id);
const char *amp_model_key(AmpModelId id);
const char *amp_model_label(AmpModelId id);
bool amp_model_available(AmpModelId id);
bool amp_model_parse(const char *value, AmpModelId &id);
AmpModelId amp_model_detect_from_packet(const uint8_t *data, uint8_t len);

struct AmpMeterSnapshot {
    const char *label = "";
    float value = 0.0f;
    uint16_t max = 0;
    const char *suffix = "";
    const char *scale[5] = {"", "", "", "", ""};
};

struct AmpStatusSnapshot {
    bool valid = false;
    AmpModelId model = AmpModelId::Unknown;
    uint8_t screen_id = 0;
    const char *screen_name = "unknown";
    uint8_t flags = 0;
    uint8_t display_context = 0;
    uint8_t op_status = 0;
    uint16_t frequency = 0;
    uint8_t sub_band = 0;
    const char *input = "?";
    const char *band = "?";
    const char *antenna = "?";
    const char *cat = "?";
    const char *out = "?";
    char swr[8] = "--.--";
    char temp[12] = "?";
    float power = 0.0f;
    float reverse = 0.0f;
    float voltage = 0.0f;
    float current = 0.0f;
    AmpMeterSnapshot power_meter;
    AmpMeterSnapshot pa_meter;
};
