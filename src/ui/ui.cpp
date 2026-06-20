/*
 * Top-level LVGL UI initialisation and screen lifecycle implementation.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */


#include "ui.h"
#include "ui_helpers.h"

#define glyph_bitmap ui_font_Helvetica_28_glyph_bitmap
#define glyph_dsc ui_font_Helvetica_28_glyph_dsc
#define cmaps ui_font_Helvetica_28_cmaps
#define font_dsc ui_font_Helvetica_28_font_dsc
#include "assets/ui_font_Helvetica_28.h"
#undef font_dsc
#undef cmaps
#undef glyph_dsc
#undef glyph_bitmap

#define glyph_bitmap ui_font_LCD_glyph_bitmap
#define glyph_dsc ui_font_LCD_glyph_dsc
#define cmaps ui_font_LCD_cmaps
#define font_dsc ui_font_LCD_font_dsc
#include "assets/ui_font_LCD.h"
#undef font_dsc
#undef cmaps
#undef glyph_dsc
#undef glyph_bitmap

#include "assets/ui_img_spe_png.h"

///////////////////// VARIABLES ////////////////////

// EVENTS
lv_obj_t * ui____initial_actions0;

// IMAGES AND IMAGE SETS

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////

///////////////////// SCREENS ////////////////////

void ui_init(void)
{
    LV_EVENT_GET_COMP_CHILD = lv_event_register_id();

    lv_disp_t * dispp = lv_display_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    ui_bootScreen_screen_init();
    ui_mainScreen_screen_init();
    ui____initial_actions0 = lv_obj_create(NULL);
    lv_disp_load_scr(ui_bootScreen);
}

void ui_destroy(void)
{
    ui_bootScreen_screen_destroy();
    ui_mainScreen_screen_destroy();
}
