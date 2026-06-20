/*
 * Arduino Giga Display Shield touch input adapter for LVGL.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "display/lvgl_giga_touch.h"

#include <lvgl.h>

static constexpr int kLogicalWidth = 800;
static constexpr int kLogicalHeight = 480;

static int16_t clamp_coord(int32_t value, int16_t low, int16_t high)
{
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return static_cast<int16_t>(value);
}

static void transformed_touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    auto *touch = static_cast<Arduino_GigaDisplayTouch *>(lv_indev_get_user_data(indev));
    GDTpoint_t points[GT911_MAX_CONTACTS];
    const uint8_t contacts = touch ? touch->getTouchPoints(points) : 0;

    if (contacts > 0) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = clamp_coord(points[0].y, 0, kLogicalWidth - 1);
        data->point.y = clamp_coord((kLogicalHeight - 1) - points[0].x, 0, kLogicalHeight - 1);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

int GigaLvglTouch::useDisplayRotation()
{
    int updated = 0;

    for (lv_indev_t *indev = lv_indev_get_next(nullptr); indev; indev = lv_indev_get_next(indev)) {
        if (lv_indev_get_type(indev) == LV_INDEV_TYPE_POINTER) {
            lv_indev_set_user_data(indev, &touch_);
            lv_indev_set_read_cb(indev, transformed_touch_read);
            updated++;
        }
    }

    if (updated == 0) {
        lv_indev_t *indev = lv_indev_create();
        if (indev) {
            lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
            lv_indev_set_user_data(indev, &touch_);
            lv_indev_set_read_cb(indev, transformed_touch_read);
            updated = 1;
        }
    }

    return updated;
}

int giga_lvgl_touch_use_display_rotation(Arduino_GigaDisplayTouch &touch)
{
    GigaLvglTouch rotated_touch(touch);
    return rotated_touch.useDisplayRotation();
}
