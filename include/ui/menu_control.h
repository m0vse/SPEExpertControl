/*
 * Menu selection helper class used by amplifier setup screens.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include "lvgl.h"

class MenuController {
public:
    /**
     * Attach the controller to a fixed array of selectable LVGL objects.
     *
     * @return true when the item array is valid.
     */
    bool begin(lv_obj_t **items, int count);

    /**
     * Move the selected style to a new item index.
     */
    void applySelection(int new_index);

    /**
     * @return The currently highlighted item index, or -1 before initialisation.
     */
    int selected() const { return selected_; }

private:
    lv_obj_t **items_ = nullptr;
    int count_ = 0;
    int selected_ = -1;
    bool syncing_ = false;
    lv_style_t checked_style_{};
};
