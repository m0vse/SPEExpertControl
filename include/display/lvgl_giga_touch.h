/*
 * Touch input adapter interface for connecting Arduino Giga Display Shield touch to LVGL.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <Arduino_GigaDisplayTouch.h>

class GigaLvglTouch {
public:
    /**
     * Create a touch adapter around the Arduino touch driver instance.
     */
    explicit GigaLvglTouch(Arduino_GigaDisplayTouch &touch) : touch_(touch) {}

    /**
     * Register the touch device with LVGL using the display's rotated coordinate system.
     *
     * @return Number of registered touch devices, or a negative value on failure.
     */
    int useDisplayRotation();

private:
    Arduino_GigaDisplayTouch &touch_;
};

/**
 * C-compatible wrapper for GigaLvglTouch::useDisplayRotation().
 */
int giga_lvgl_touch_use_display_rotation(Arduino_GigaDisplayTouch &touch);
