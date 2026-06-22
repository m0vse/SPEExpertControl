/*
 * LCD idle blanking policy and touch wake handling.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "ui/screensaver.h"

#include <Arduino_GigaDisplay.h>

static GigaDisplayBacklight backlight;
static uint16_t timeout_min = 0;
static unsigned long last_activity_ms = 0;
static bool active = false;
static bool suppress_touch_until_release = false;
static bool backlight_started = false;

static unsigned long timeout_ms(void)
{
    return static_cast<unsigned long>(timeout_min) * 60UL * 1000UL;
}

void screensaver_begin(uint16_t timeout_minutes)
{
    backlight.begin(100);
    backlight_started = true;
    timeout_min = timeout_minutes;
    last_activity_ms = millis();
    active = false;
    suppress_touch_until_release = false;
}

void screensaver_set_timeout_minutes(uint16_t timeout_minutes)
{
    timeout_min = timeout_minutes;
    if (timeout_min == 0) {
        screensaver_wake();
    } else {
        screensaver_note_activity();
    }
}

uint16_t screensaver_timeout_minutes(void)
{
    return timeout_min;
}

void screensaver_service(unsigned long now)
{
    if (timeout_min == 0 || active) {
        return;
    }

    if (now - last_activity_ms >= timeout_ms()) {
        backlight.off();
        active = true;
        suppress_touch_until_release = false;
    }
}

void screensaver_note_activity(void)
{
    last_activity_ms = millis();
    if (active) {
        screensaver_wake();
    }
}

bool screensaver_note_touch(unsigned long now)
{
    if (active) {
        if (!backlight_started) {
            backlight.begin(100);
            backlight_started = true;
        } else {
            backlight.set(100);
        }
        active = false;
        last_activity_ms = now;
        suppress_touch_until_release = true;
        return false;
    }

    if (suppress_touch_until_release) {
        return false;
    }

    last_activity_ms = now;
    return true;
}

void screensaver_note_release(void)
{
    suppress_touch_until_release = false;
}

void screensaver_wake(void)
{
    if (!backlight_started) {
        backlight.begin(100);
        backlight_started = true;
    } else {
        backlight.set(100);
    }
    active = false;
    last_activity_ms = millis();
}

bool screensaver_is_active(void)
{
    return active;
}
