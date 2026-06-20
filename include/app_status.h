/*
 * Application status export API used to expose amplifier state as JSON.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <Print.h>

/**
 * Write the latest amplifier/UI state as a JSON object to any Arduino Print stream.
 *
 * The caller owns the surrounding JSON context and stream lifetime.
 */
void app_status_print_json(Print &out);
