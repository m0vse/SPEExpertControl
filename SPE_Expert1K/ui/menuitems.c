/*
* This is an unfortunate necessity of using the "free" version of SquareLine Studio which is limited to 150 items.
*
* I created all menu items in SquareLine and then copied them here. This does mean that it isn't possible to modify the menu items in
* SquareLine, but they aren't likely to need changing much once defined, and this does lend itself to better management (array of objects).
*/
#include "lvgl.h"
#include <ui.h>


// setupOptions
lv_obj_t * ui_setupAntenna = NULL;
lv_obj_t * ui_setupContest = NULL;
lv_obj_t * ui_setupBeep = NULL;
lv_obj_t * ui_setupStart = NULL;
lv_obj_t * ui_setupTemp = NULL;
lv_obj_t * ui_setupQuit = NULL;
lv_obj_t * ui_setupCat = NULL;
lv_obj_t * ui_setupManualTune = NULL;
lv_obj_t * ui_setupBacklight = NULL;

// setupAntContainer1
lv_obj_t * ui_setup160AntLabel = NULL;
lv_obj_t * ui_setup30AntLabel = NULL;
lv_obj_t * ui_setup40AntLabel = NULL;
lv_obj_t * ui_setup80AntLabel = NULL;
lv_obj_t * ui_setup160Ant1 = NULL;
lv_obj_t * ui_setup160Ant2 = NULL;
lv_obj_t * ui_setup80Ant1 = NULL;
lv_obj_t * ui_setup80Ant2 = NULL;
lv_obj_t * ui_setup40Ant1 = NULL;
lv_obj_t * ui_setup40Ant2 = NULL;
lv_obj_t * ui_setup30Ant1 = NULL;
lv_obj_t * ui_setup30Ant2 = NULL;
// setupAntContainer2
lv_obj_t * ui_setup20AntLabel = NULL;
lv_obj_t * ui_setup12AntLabel = NULL;
lv_obj_t * ui_setup15AntLabel = NULL;
lv_obj_t * ui_setup17AntLabel = NULL;
lv_obj_t * ui_setup20Ant1 = NULL;
lv_obj_t * ui_setup20Ant2 = NULL;
lv_obj_t * ui_setup17Ant1 = NULL;
lv_obj_t * ui_setup17Ant2 = NULL;
lv_obj_t * ui_setup15Ant1 = NULL;
lv_obj_t * ui_setup15Ant2 = NULL;
lv_obj_t * ui_setup12Ant1 = NULL;
lv_obj_t * ui_setup12Ant2 = NULL;
// setupAntContainer3
lv_obj_t * ui_setup10AntLabel = NULL;
lv_obj_t * ui_setup6AntLabel = NULL;
lv_obj_t * ui_setup10Ant1 = NULL;
lv_obj_t * ui_setup10Ant2 = NULL;
lv_obj_t * ui_setup6Ant1 = NULL;
lv_obj_t * ui_setup6Ant2 = NULL;
lv_obj_t * ui_setupAntSave = NULL;

void setupMenus(void)
{
    // I will probably convert these to arrays, but this is just a direct copy of SquareLine

    // First the main setup options menu
    ui_setupAntenna = lv_label_create(ui_setupOptions);
    lv_obj_set_width(ui_setupAntenna, 260);
    lv_obj_set_height(ui_setupAntenna, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupAntenna, 0);
    lv_obj_set_y(ui_setupAntenna, 25);
    lv_label_set_text(ui_setupAntenna, "ANTENNA");
    lv_obj_set_style_text_align(ui_setupAntenna, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setupAntenna, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupAntenna, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupAntenna, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupAntenna, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupAntenna, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupContest = lv_label_create(ui_setupOptions);
    lv_obj_set_width(ui_setupContest, 320);
    lv_obj_set_height(ui_setupContest, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupContest, 285);
    lv_obj_set_y(ui_setupContest, 25);
    lv_label_set_text(ui_setupContest, "CONTEST Off");
    lv_obj_set_style_text_align(ui_setupContest, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setupContest, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupContest, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupContest, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupContest, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupContest, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupBeep = lv_label_create(ui_setupOptions);
    lv_obj_set_width(ui_setupBeep, 320);
    lv_obj_set_height(ui_setupBeep, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupBeep, 285);
    lv_obj_set_y(ui_setupBeep, 60);
    lv_label_set_text(ui_setupBeep, "BEEP    Off");
    lv_obj_set_style_text_align(ui_setupBeep, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setupBeep, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupBeep, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupBeep, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupBeep, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupBeep, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupStart = lv_label_create(ui_setupOptions);
    lv_obj_set_width(ui_setupStart, 320);
    lv_obj_set_height(ui_setupStart, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupStart, 285);
    lv_obj_set_y(ui_setupStart, 95);
    lv_label_set_text(ui_setupStart, "START   Standby");
    lv_obj_set_style_text_align(ui_setupStart, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setupStart, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupStart, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupStart, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupStart, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupStart, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupTemp = lv_label_create(ui_setupOptions);
    lv_obj_set_width(ui_setupTemp, 320);
    lv_obj_set_height(ui_setupTemp, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupTemp, 285);
    lv_obj_set_y(ui_setupTemp, 130);
    lv_label_set_text(ui_setupTemp, "TEMP    °C");
    lv_obj_set_style_text_align(ui_setupTemp, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setupTemp, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupTemp, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupQuit = lv_label_create(ui_setupOptions);
    lv_obj_set_width(ui_setupQuit, 100);
    lv_obj_set_height(ui_setupQuit, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupQuit, 660);
    lv_obj_set_y(ui_setupQuit, 25);
    lv_label_set_text(ui_setupQuit, "QUIT");
    lv_obj_set_style_text_align(ui_setupQuit, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setupQuit, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupQuit, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupQuit, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupQuit, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupQuit, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupCat = lv_label_create(ui_setupOptions);
    lv_obj_set_width(ui_setupCat, 260);
    lv_obj_set_height(ui_setupCat, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupCat, 0);
    lv_obj_set_y(ui_setupCat, 60);
    lv_label_set_text(ui_setupCat, "CAT");
    lv_obj_set_style_text_align(ui_setupCat, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setupCat, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupCat, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupCat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupCat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupCat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupManualTune = lv_label_create(ui_setupOptions);
    lv_obj_set_width(ui_setupManualTune, 260);
    lv_obj_set_height(ui_setupManualTune, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupManualTune, 0);
    lv_obj_set_y(ui_setupManualTune, 95);
    lv_label_set_text(ui_setupManualTune, "MANUAL TUNE");
    lv_obj_set_style_text_align(ui_setupManualTune, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setupManualTune, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupManualTune, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupManualTune, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupManualTune, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupManualTune, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupBacklight = lv_label_create(ui_setupOptions);
    lv_obj_set_width(ui_setupBacklight, 260);
    lv_obj_set_height(ui_setupBacklight, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupBacklight, 0);
    lv_obj_set_y(ui_setupBacklight, 130);
    lv_label_set_text(ui_setupBacklight, "BACKLIGHT");
    lv_obj_set_style_text_align(ui_setupBacklight, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setupBacklight, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupBacklight, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupBacklight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupBacklight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupBacklight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // The 3 containers of the Antenna menu    
    // Container1
    ui_setup160AntLabel = lv_label_create(ui_setupAntContainer1);
    lv_obj_set_width(ui_setup160AntLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_setup160AntLabel, LV_SIZE_CONTENT);    /// 1
    lv_label_set_text(ui_setup160AntLabel, "160 m:");
    lv_obj_set_style_text_font(ui_setup160AntLabel, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup30AntLabel = lv_label_create(ui_setupAntContainer1);
    lv_obj_set_width(ui_setup30AntLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_setup30AntLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup30AntLabel, 0);
    lv_obj_set_y(ui_setup30AntLabel, 90);
    lv_label_set_text(ui_setup30AntLabel, " 30 m:");
    lv_obj_set_style_text_font(ui_setup30AntLabel, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup40AntLabel = lv_label_create(ui_setupAntContainer1);
    lv_obj_set_width(ui_setup40AntLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_setup40AntLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup40AntLabel, 0);
    lv_obj_set_y(ui_setup40AntLabel, 60);
    lv_label_set_text(ui_setup40AntLabel, " 40 m:");
    lv_obj_set_style_text_font(ui_setup40AntLabel, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup80AntLabel = lv_label_create(ui_setupAntContainer1);
    lv_obj_set_width(ui_setup80AntLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_setup80AntLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup80AntLabel, 0);
    lv_obj_set_y(ui_setup80AntLabel, 30);
    lv_label_set_text(ui_setup80AntLabel, " 80 m:");
    lv_obj_set_style_text_font(ui_setup80AntLabel, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup160Ant1 = lv_label_create(ui_setupAntContainer1);
    lv_obj_set_width(ui_setup160Ant1, 55);
    lv_obj_set_height(ui_setup160Ant1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup160Ant1, 120);
    lv_obj_set_y(ui_setup160Ant1, 0);
    lv_label_set_text(ui_setup160Ant1, " 2");
    lv_obj_set_style_text_align(ui_setup160Ant1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup160Ant1, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup160Ant2 = lv_label_create(ui_setupAntContainer1);
    lv_obj_set_width(ui_setup160Ant2, 55);
    lv_obj_set_height(ui_setup160Ant2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup160Ant2, 185);
    lv_obj_set_y(ui_setup160Ant2, 0);
    lv_label_set_long_mode(ui_setup160Ant2, LV_LABEL_LONG_CLIP);
    lv_label_set_text(ui_setup160Ant2, "NO");
    lv_obj_set_style_text_align(ui_setup160Ant2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup160Ant2, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup80Ant1 = lv_label_create(ui_setupAntContainer1);
    lv_obj_set_width(ui_setup80Ant1, 55);
    lv_obj_set_height(ui_setup80Ant1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup80Ant1, 120);
    lv_obj_set_y(ui_setup80Ant1, 30);
    lv_label_set_text(ui_setup80Ant1, " 2");
    lv_obj_set_style_text_align(ui_setup80Ant1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup80Ant1, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup80Ant2 = lv_label_create(ui_setupAntContainer1);
    lv_obj_set_width(ui_setup80Ant2, 55);
    lv_obj_set_height(ui_setup80Ant2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup80Ant2, 185);
    lv_obj_set_y(ui_setup80Ant2, 30);
    lv_label_set_long_mode(ui_setup80Ant2, LV_LABEL_LONG_CLIP);
    lv_label_set_text(ui_setup80Ant2, "NO");
    lv_obj_set_style_text_align(ui_setup80Ant2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup80Ant2, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup40Ant1 = lv_label_create(ui_setupAntContainer1);
    lv_obj_set_width(ui_setup40Ant1, 55);
    lv_obj_set_height(ui_setup40Ant1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup40Ant1, 120);
    lv_obj_set_y(ui_setup40Ant1, 60);
    lv_label_set_text(ui_setup40Ant1, " 2");
    lv_obj_set_style_text_align(ui_setup40Ant1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup40Ant1, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup40Ant2 = lv_label_create(ui_setupAntContainer1);
    lv_obj_set_width(ui_setup40Ant2, 55);
    lv_obj_set_height(ui_setup40Ant2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup40Ant2, 185);
    lv_obj_set_y(ui_setup40Ant2, 60);
    lv_label_set_long_mode(ui_setup40Ant2, LV_LABEL_LONG_CLIP);
    lv_label_set_text(ui_setup40Ant2, "NO");
    lv_obj_set_style_text_align(ui_setup40Ant2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup40Ant2, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup30Ant1 = lv_label_create(ui_setupAntContainer1);
    lv_obj_set_width(ui_setup30Ant1, 55);
    lv_obj_set_height(ui_setup30Ant1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup30Ant1, 120);
    lv_obj_set_y(ui_setup30Ant1, 90);
    lv_label_set_text(ui_setup30Ant1, " 2");
    lv_obj_set_style_text_align(ui_setup30Ant1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup30Ant1, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup30Ant2 = lv_label_create(ui_setupAntContainer1);
    lv_obj_set_width(ui_setup30Ant2, 55);
    lv_obj_set_height(ui_setup30Ant2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup30Ant2, 185);
    lv_obj_set_y(ui_setup30Ant2, 90);
    lv_label_set_long_mode(ui_setup30Ant2, LV_LABEL_LONG_CLIP);
    lv_label_set_text(ui_setup30Ant2, "NO");
    lv_obj_set_style_text_align(ui_setup30Ant2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup30Ant2, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Container2
       ui_setup20AntLabel = lv_label_create(ui_setupAntContainer2);
    lv_obj_set_width(ui_setup20AntLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_setup20AntLabel, LV_SIZE_CONTENT);    /// 1
    lv_label_set_text(ui_setup20AntLabel, "20 m:");
    lv_obj_set_style_text_font(ui_setup20AntLabel, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup12AntLabel = lv_label_create(ui_setupAntContainer2);
    lv_obj_set_width(ui_setup12AntLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_setup12AntLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup12AntLabel, 0);
    lv_obj_set_y(ui_setup12AntLabel, 90);
    lv_label_set_text(ui_setup12AntLabel, "12 m:");
    lv_obj_set_style_text_font(ui_setup12AntLabel, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup15AntLabel = lv_label_create(ui_setupAntContainer2);
    lv_obj_set_width(ui_setup15AntLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_setup15AntLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup15AntLabel, 0);
    lv_obj_set_y(ui_setup15AntLabel, 60);
    lv_label_set_text(ui_setup15AntLabel, "15 m:");
    lv_obj_set_style_text_font(ui_setup15AntLabel, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup17AntLabel = lv_label_create(ui_setupAntContainer2);
    lv_obj_set_width(ui_setup17AntLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_setup17AntLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup17AntLabel, 0);
    lv_obj_set_y(ui_setup17AntLabel, 30);
    lv_label_set_text(ui_setup17AntLabel, "17 m:");
    lv_obj_set_style_text_font(ui_setup17AntLabel, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup20Ant1 = lv_label_create(ui_setupAntContainer2);
    lv_obj_set_width(ui_setup20Ant1, 55);
    lv_obj_set_height(ui_setup20Ant1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup20Ant1, 100);
    lv_obj_set_y(ui_setup20Ant1, 0);
    lv_label_set_text(ui_setup20Ant1, " 2");
    lv_obj_set_style_text_align(ui_setup20Ant1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup20Ant1, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup20Ant2 = lv_label_create(ui_setupAntContainer2);
    lv_obj_set_width(ui_setup20Ant2, 55);
    lv_obj_set_height(ui_setup20Ant2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup20Ant2, 165);
    lv_obj_set_y(ui_setup20Ant2, 0);
    lv_label_set_long_mode(ui_setup20Ant2, LV_LABEL_LONG_CLIP);
    lv_label_set_text(ui_setup20Ant2, "NO");
    lv_obj_set_style_text_align(ui_setup20Ant2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup20Ant2, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup17Ant1 = lv_label_create(ui_setupAntContainer2);
    lv_obj_set_width(ui_setup17Ant1, 55);
    lv_obj_set_height(ui_setup17Ant1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup17Ant1, 100);
    lv_obj_set_y(ui_setup17Ant1, 30);
    lv_label_set_text(ui_setup17Ant1, " 2");
    lv_obj_set_style_text_align(ui_setup17Ant1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup17Ant1, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup17Ant2 = lv_label_create(ui_setupAntContainer2);
    lv_obj_set_width(ui_setup17Ant2, 55);
    lv_obj_set_height(ui_setup17Ant2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup17Ant2, 165);
    lv_obj_set_y(ui_setup17Ant2, 30);
    lv_label_set_long_mode(ui_setup17Ant2, LV_LABEL_LONG_CLIP);
    lv_label_set_text(ui_setup17Ant2, "NO");
    lv_obj_set_style_text_align(ui_setup17Ant2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup17Ant2, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup15Ant1 = lv_label_create(ui_setupAntContainer2);
    lv_obj_set_width(ui_setup15Ant1, 55);
    lv_obj_set_height(ui_setup15Ant1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup15Ant1, 100);
    lv_obj_set_y(ui_setup15Ant1, 60);
    lv_label_set_text(ui_setup15Ant1, " 2");
    lv_obj_set_style_text_align(ui_setup15Ant1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup15Ant1, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup15Ant2 = lv_label_create(ui_setupAntContainer2);
    lv_obj_set_width(ui_setup15Ant2, 55);
    lv_obj_set_height(ui_setup15Ant2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup15Ant2, 165);
    lv_obj_set_y(ui_setup15Ant2, 60);
    lv_label_set_long_mode(ui_setup15Ant2, LV_LABEL_LONG_CLIP);
    lv_label_set_text(ui_setup15Ant2, "NO");
    lv_obj_set_style_text_align(ui_setup15Ant2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup15Ant2, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup12Ant1 = lv_label_create(ui_setupAntContainer2);
    lv_obj_set_width(ui_setup12Ant1, 55);
    lv_obj_set_height(ui_setup12Ant1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup12Ant1, 100);
    lv_obj_set_y(ui_setup12Ant1, 90);
    lv_label_set_text(ui_setup12Ant1, " 2");
    lv_obj_set_style_text_align(ui_setup12Ant1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup12Ant1, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup12Ant2 = lv_label_create(ui_setupAntContainer2);
    lv_obj_set_width(ui_setup12Ant2, 55);
    lv_obj_set_height(ui_setup12Ant2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup12Ant2, 165);
    lv_obj_set_y(ui_setup12Ant2, 90);
    lv_label_set_long_mode(ui_setup12Ant2, LV_LABEL_LONG_CLIP);
    lv_label_set_text(ui_setup12Ant2, "NO");
    lv_obj_set_style_text_align(ui_setup12Ant2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup12Ant2, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Container3
    ui_setup10AntLabel = lv_label_create(ui_setupAntContainer3);
    lv_obj_set_width(ui_setup10AntLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_setup10AntLabel, LV_SIZE_CONTENT);    /// 1
    lv_label_set_text(ui_setup10AntLabel, "10 m:");
    lv_obj_set_style_text_font(ui_setup10AntLabel, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup6AntLabel = lv_label_create(ui_setupAntContainer3);
    lv_obj_set_width(ui_setup6AntLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_setup6AntLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup6AntLabel, 0);
    lv_obj_set_y(ui_setup6AntLabel, 30);
    lv_label_set_text(ui_setup6AntLabel, " 6 m:");
    lv_obj_set_style_text_font(ui_setup6AntLabel, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup10Ant1 = lv_label_create(ui_setupAntContainer3);
    lv_obj_set_width(ui_setup10Ant1, 55);
    lv_obj_set_height(ui_setup10Ant1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup10Ant1, 100);
    lv_obj_set_y(ui_setup10Ant1, 0);
    lv_label_set_text(ui_setup10Ant1, " 2");
    lv_obj_set_style_text_align(ui_setup10Ant1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup10Ant1, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup10Ant2 = lv_label_create(ui_setupAntContainer3);
    lv_obj_set_width(ui_setup10Ant2, 55);
    lv_obj_set_height(ui_setup10Ant2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup10Ant2, 165);
    lv_obj_set_y(ui_setup10Ant2, 0);
    lv_label_set_long_mode(ui_setup10Ant2, LV_LABEL_LONG_CLIP);
    lv_label_set_text(ui_setup10Ant2, "NO");
    lv_obj_set_style_text_align(ui_setup10Ant2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup10Ant2, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup6Ant1 = lv_label_create(ui_setupAntContainer3);
    lv_obj_set_width(ui_setup6Ant1, 55);
    lv_obj_set_height(ui_setup6Ant1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup6Ant1, 100);
    lv_obj_set_y(ui_setup6Ant1, 30);
    lv_label_set_text(ui_setup6Ant1, " 2");
    lv_obj_set_style_text_align(ui_setup6Ant1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup6Ant1, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setup6Ant2 = lv_label_create(ui_setupAntContainer3);
    lv_obj_set_width(ui_setup6Ant2, 55);
    lv_obj_set_height(ui_setup6Ant2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setup6Ant2, 165);
    lv_obj_set_y(ui_setup6Ant2, 30);
    lv_label_set_long_mode(ui_setup6Ant2, LV_LABEL_LONG_CLIP);
    lv_label_set_text(ui_setup6Ant2, "NO");
    lv_obj_set_style_text_align(ui_setup6Ant2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setup6Ant2, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupAntSave = lv_label_create(ui_setupAntContainer3);
    lv_obj_set_width(ui_setupAntSave, 110);
    lv_obj_set_height(ui_setupAntSave, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupAntSave, 110);
    lv_obj_set_y(ui_setupAntSave, 90);
    lv_label_set_long_mode(ui_setupAntSave, LV_LABEL_LONG_CLIP);
    lv_label_set_text(ui_setupAntSave, "SAVE");
    lv_obj_set_style_text_align(ui_setupAntSave, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setupAntSave, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupAntSave, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupAntSave, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupAntSave, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupAntSave, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

}