/*
 * LVGL canvas renderer for modern SPE 40x8 LCD snapshots.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <lvgl.h>

void modern_lcd_canvas_create(lv_obj_t *parent);
void modern_lcd_canvas_hide();
void modern_lcd_canvas_show();
bool modern_lcd_canvas_render(const char *lcd_cells_hex, const char *lcd_attrs_hex);
