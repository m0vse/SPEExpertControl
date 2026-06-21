/*
 * Display adapter interface for connecting Arduino Giga Display Shield video to LVGL.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

class GigaLvglDisplay {
public:
    /**
     * Initialise the Giga Display Shield video path and attach it to LVGL.
     *
     * @return 0 on success, non-zero on display initialisation failure.
     */
    int begin();
};

/**
 * C-compatible wrapper for GigaLvglDisplay::begin().
 */
int giga_lvgl_display_begin(void);

/**
 * Flip the logical display by 180 degrees for upside-down mounting.
 */
void giga_lvgl_display_set_flipped(bool flipped);

/**
 * Return whether the display is currently flipped.
 */
bool giga_lvgl_display_is_flipped(void);
