/*
 * Small helper class for moving LVGL menu selection highlights.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "ui/menu_control.h"

#include <Arduino.h>

bool MenuController::begin(lv_obj_t **items, int count) {
    if (!items || count <= 0) return false;

    items_ = items;
    count_ = count;
    selected_ = -1;
    syncing_ = false;

    lv_style_init(&checked_style_);
    lv_style_set_bg_opa(&checked_style_, LV_OPA_COVER);
    lv_style_set_bg_color(&checked_style_, lv_color_black());
    lv_style_set_text_color(&checked_style_, lv_color_white());

    for (int i = 0; i < count; ++i) {
        lv_obj_t *it = items[i];
        if (!it || !lv_obj_is_valid(it)) {
            Serial.println("Invalid menu item, cannot continue");
            return false;
        }

        lv_obj_add_style(it, &checked_style_, LV_PART_MAIN | LV_STATE_CHECKED);
    }
    return true;
}

void MenuController::applySelection(int new_index) {
    if (syncing_) return;
    syncing_ = true;

    if (new_index < -1) new_index = -1;
    if (new_index >= count_) new_index = count_ - 1;

    if (new_index == selected_) {
        syncing_ = false;
        return;
    }

    if (selected_ >= 0 && selected_ < count_) {
        lv_obj_clear_state(items_[selected_], LV_STATE_CHECKED);
    }
    selected_ = new_index;

    if (selected_ >= 0) {
        lv_obj_add_state(items_[selected_], LV_STATE_CHECKED);
    }
    syncing_ = false;
}
