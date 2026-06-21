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
#include "models/spe_expert1k/expertpackets.h"
#include "models/spe_expert1k/protocol.h"

struct AppStatusSnapshot {
    bool valid = false;
    ExpertScreen screen = BootMessage;
    Expert_Packet status{};
    Expert_Packet web_cat_snapshot{};
    unsigned long web_cat_snapshot_until = 0;
    AmpStatusSnapshot amp{};
    uint32_t sequence = 0;
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
