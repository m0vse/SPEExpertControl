/*
 * Arduino Giga Display Shield video/display adapter for LVGL.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "display/lvgl_giga_display.h"

#include <Arduino.h>
#include <Arduino_GigaDisplay_GFX.h>
#include <lvgl.h>
#include <stdlib.h>

static constexpr int kLogicalWidth = 800;
static constexpr int kLogicalHeight = 480;
static constexpr int kPhysicalWidth = 480;
static constexpr int kBufferRows = 48;

static GigaDisplay_GFX gfx;
static lv_color_t *draw_buffer = nullptr;
static bool display_flipped = false;

static uint32_t arduino_tick(void)
{
    return millis();
}

static void flush_display(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    uint16_t *framebuffer = gfx.getBuffer();
    const uint16_t *source = reinterpret_cast<const uint16_t *>(px_map);

    if (framebuffer) {
        for (int32_t y = area->y1; y <= area->y2; ++y) {
            for (int32_t x = area->x1; x <= area->x2; ++x) {
                const int32_t physical_x = display_flipped ? y : kLogicalHeight - 1 - y;
                const int32_t physical_y = display_flipped ? kLogicalWidth - 1 - x : x;
                framebuffer[(physical_y * kPhysicalWidth) + physical_x] = *source++;
            }
        }

        if (lv_display_flush_is_last(display)) {
            gfx.endWrite();
        }
    }

    lv_display_flush_ready(display);
}

int GigaLvglDisplay::begin()
{
    gfx.begin();

    if (!gfx.getBuffer()) {
        return 2;
    }

    lv_init();
    lv_tick_set_cb(arduino_tick);

    draw_buffer = static_cast<lv_color_t *>(malloc(kLogicalWidth * kBufferRows * sizeof(lv_color_t)));
    if (!draw_buffer) {
        return 3;
    }

    lv_display_t *display = lv_display_create(kLogicalWidth, kLogicalHeight);
    if (!display) {
        return 4;
    }

    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, draw_buffer, nullptr, kLogicalWidth * kBufferRows * sizeof(lv_color_t),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush_display);

    return 0;
}

int giga_lvgl_display_begin(void)
{
    static GigaLvglDisplay display;
    return display.begin();
}

void giga_lvgl_display_set_flipped(bool flipped)
{
    display_flipped = flipped;
    if (lv_display_get_default()) {
        lv_obj_invalidate(lv_screen_active());
    }
}

bool giga_lvgl_display_is_flipped(void)
{
    return display_flipped;
}
