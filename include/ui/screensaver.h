/*
 * LCD idle blanking policy and touch wake handling.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <Arduino.h>

void screensaver_begin(uint16_t timeout_minutes);
void screensaver_set_timeout_minutes(uint16_t timeout_minutes);
uint16_t screensaver_timeout_minutes(void);
void screensaver_service(unsigned long now);
void screensaver_note_activity(void);
bool screensaver_note_touch(unsigned long now);
void screensaver_note_release(void);
void screensaver_wake(void);
bool screensaver_is_active(void);
