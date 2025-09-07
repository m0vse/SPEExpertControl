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
// setupCatOptions
lv_obj_t * ui_setupCatSpe = NULL;
lv_obj_t * ui_setupCatIcom = NULL;
lv_obj_t * ui_setupCatKenwood = NULL;
lv_obj_t * ui_setupCatYaesu = NULL;
lv_obj_t * ui_setupCatTenTec = NULL;
lv_obj_t * ui_setupCatFlexRadio = NULL;
lv_obj_t * ui_setupCatRs232 = NULL;
lv_obj_t * ui_setupCatNone = NULL;
// setupYaesuOptions
lv_obj_t * ui_setupYaesuFT100 = NULL;
lv_obj_t * ui_setupYaesuFT757 = NULL;
lv_obj_t * ui_setupYaesuFT817 = NULL;
lv_obj_t * ui_setupYaesuFT840 = NULL;
lv_obj_t * ui_setupYaesuFT897 = NULL;
lv_obj_t * ui_setupYaesuFT900 = NULL;
lv_obj_t * ui_setupYaesuFT920 = NULL;
lv_obj_t * ui_setupYaesuFT990 = NULL;
lv_obj_t * ui_setupYaesuFT1000 = NULL;
lv_obj_t * ui_setupYaesuFT1000MP1 = NULL;
lv_obj_t * ui_setupYaesuFT1000MP2 = NULL;
lv_obj_t * ui_setupYaesuFT1000MP3 = NULL;
lv_obj_t * ui_setupYaesuFT2000 = NULL;
lv_obj_t * ui_setupYaesuFT9000 = NULL;
lv_obj_t * ui_setupYaesuBandData = NULL;
// setupIcomOptions
lv_obj_t * ui_setupIcomCiv = NULL;
lv_obj_t * ui_setupIcomVoltage = NULL;
// setupTenTecOptions
lv_obj_t * ui_setupTenTecOmni = NULL;
lv_obj_t * ui_setupTenTecOrion = NULL;
lv_obj_t * ui_setupTenTecJupiter = NULL;
lv_obj_t * ui_setupTenTecArgonaut = NULL;
// setupBaudRateOptions
lv_obj_t * ui_setupBaud1200 = NULL;
lv_obj_t * ui_setupBaud2400 = NULL;
lv_obj_t * ui_setupBaud4800 = NULL;
lv_obj_t * ui_setupBaud9600 = NULL;

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

    // CAT Options
    ui_setupCatSpe = lv_label_create(ui_setupCatOptions);
    lv_obj_set_width(ui_setupCatSpe, 230);
    lv_obj_set_height(ui_setupCatSpe, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupCatSpe, 0);
    lv_obj_set_y(ui_setupCatSpe, 45);
    lv_label_set_text(ui_setupCatSpe, "SPE");
    lv_obj_set_style_text_font(ui_setupCatSpe, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupCatSpe, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupCatSpe, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupCatSpe, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupCatSpe, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupCatIcom = lv_label_create(ui_setupCatOptions);
    lv_obj_set_width(ui_setupCatIcom, 230);
    lv_obj_set_height(ui_setupCatIcom, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupCatIcom, 0);
    lv_obj_set_y(ui_setupCatIcom, 75);
    lv_label_set_text(ui_setupCatIcom, "ICOM");
    lv_obj_set_style_text_font(ui_setupCatIcom, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupCatIcom, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupCatIcom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupCatIcom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupCatIcom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupCatKenwood = lv_label_create(ui_setupCatOptions);
    lv_obj_set_width(ui_setupCatKenwood, 230);
    lv_obj_set_height(ui_setupCatKenwood, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupCatKenwood, 0);
    lv_obj_set_y(ui_setupCatKenwood, 105);
    lv_label_set_text(ui_setupCatKenwood, "KENWOOD");
    lv_obj_set_style_text_font(ui_setupCatKenwood, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupCatKenwood, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupCatKenwood, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupCatKenwood, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupCatKenwood, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupCatYaesu = lv_label_create(ui_setupCatOptions);
    lv_obj_set_width(ui_setupCatYaesu, 230);
    lv_obj_set_height(ui_setupCatYaesu, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupCatYaesu, 245);
    lv_obj_set_y(ui_setupCatYaesu, 45);
    lv_label_set_text(ui_setupCatYaesu, "YAESU");
    lv_obj_set_style_text_font(ui_setupCatYaesu, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupCatYaesu, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupCatYaesu, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupCatYaesu, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupCatYaesu, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupCatTenTec = lv_label_create(ui_setupCatOptions);
    lv_obj_set_width(ui_setupCatTenTec, 230);
    lv_obj_set_height(ui_setupCatTenTec, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupCatTenTec, 245);
    lv_obj_set_y(ui_setupCatTenTec, 75);
    lv_label_set_text(ui_setupCatTenTec, "TEN-TEC");
    lv_obj_set_style_text_font(ui_setupCatTenTec, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupCatTenTec, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupCatTenTec, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupCatTenTec, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupCatTenTec, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupCatFlexRadio = lv_label_create(ui_setupCatOptions);
    lv_obj_set_width(ui_setupCatFlexRadio, 230);
    lv_obj_set_height(ui_setupCatFlexRadio, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupCatFlexRadio, 245);
    lv_obj_set_y(ui_setupCatFlexRadio, 105);
    lv_label_set_text(ui_setupCatFlexRadio, "FLEX-RADIO");
    lv_obj_set_style_text_font(ui_setupCatFlexRadio, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupCatFlexRadio, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupCatFlexRadio, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupCatFlexRadio, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupCatFlexRadio, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupCatRs232 = lv_label_create(ui_setupCatOptions);
    lv_obj_set_width(ui_setupCatRs232, 230);
    lv_obj_set_height(ui_setupCatRs232, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupCatRs232, 530);
    lv_obj_set_y(ui_setupCatRs232, 45);
    lv_label_set_text(ui_setupCatRs232, "RS-232");
    lv_obj_set_style_text_font(ui_setupCatRs232, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupCatRs232, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupCatRs232, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupCatRs232, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupCatRs232, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupCatNone = lv_label_create(ui_setupCatOptions);
    lv_obj_set_width(ui_setupCatNone, 230);
    lv_obj_set_height(ui_setupCatNone, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupCatNone, 530);
    lv_obj_set_y(ui_setupCatNone, 75);
    lv_label_set_text(ui_setupCatNone, "NONE");
    lv_obj_set_style_text_font(ui_setupCatNone, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupCatNone, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupCatNone, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupCatNone, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupCatNone, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Yaesu Options
    ui_setupYaesuFT100 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT100, 230);
    lv_obj_set_height(ui_setupYaesuFT100, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT100, 0);
    lv_obj_set_y(ui_setupYaesuFT100, 25);
    lv_label_set_text(ui_setupYaesuFT100, "FT 100");
    lv_obj_set_style_text_font(ui_setupYaesuFT100, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT100, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT100, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT100, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT100, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuFT757 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT757, 230);
    lv_obj_set_height(ui_setupYaesuFT757, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT757, 0);
    lv_obj_set_y(ui_setupYaesuFT757, 53);
    lv_label_set_text(ui_setupYaesuFT757, "FT 757");
    lv_obj_set_style_text_font(ui_setupYaesuFT757, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT757, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT757, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT757, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT757, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuFT817 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT817, 230);
    lv_obj_set_height(ui_setupYaesuFT817, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT817, 0);
    lv_obj_set_y(ui_setupYaesuFT817, 81);
    lv_label_set_text(ui_setupYaesuFT817, "FT 817/847");
    lv_obj_set_style_text_font(ui_setupYaesuFT817, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT817, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT817, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT817, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT817, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuFT840 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT840, 230);
    lv_obj_set_height(ui_setupYaesuFT840, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT840, 0);
    lv_obj_set_y(ui_setupYaesuFT840, 109);
    lv_label_set_text(ui_setupYaesuFT840, "FT 840/890");
    lv_obj_set_style_text_font(ui_setupYaesuFT840, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT840, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT840, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT840, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT840, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuFT897 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT897, 230);
    lv_obj_set_height(ui_setupYaesuFT897, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT897, 0);
    lv_obj_set_y(ui_setupYaesuFT897, 137);
    lv_label_set_text(ui_setupYaesuFT897, "FT 897");
    lv_obj_set_style_text_font(ui_setupYaesuFT897, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT897, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT897, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT897, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT897, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuFT900 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT900, 230);
    lv_obj_set_height(ui_setupYaesuFT900, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT900, 245);
    lv_obj_set_y(ui_setupYaesuFT900, 25);
    lv_label_set_text(ui_setupYaesuFT900, "FT 900");
    lv_obj_set_style_text_font(ui_setupYaesuFT900, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT900, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT900, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT900, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT900, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuFT920 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT920, 230);
    lv_obj_set_height(ui_setupYaesuFT920, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT920, 245);
    lv_obj_set_y(ui_setupYaesuFT920, 53);
    lv_label_set_text(ui_setupYaesuFT920, "FT 920");
    lv_obj_set_style_text_font(ui_setupYaesuFT920, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT920, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT920, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT920, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT920, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuFT990 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT990, 230);
    lv_obj_set_height(ui_setupYaesuFT990, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT990, 245);
    lv_obj_set_y(ui_setupYaesuFT990, 81);
    lv_label_set_text(ui_setupYaesuFT990, "FT 990");
    lv_obj_set_style_text_font(ui_setupYaesuFT990, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT990, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT990, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT990, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT990, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuFT1000 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT1000, 230);
    lv_obj_set_height(ui_setupYaesuFT1000, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT1000, 245);
    lv_obj_set_y(ui_setupYaesuFT1000, 109);
    lv_label_set_text(ui_setupYaesuFT1000, "FT1000");
    lv_obj_set_style_text_font(ui_setupYaesuFT1000, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT1000, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT1000, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT1000, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT1000, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuFT1000MP1 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT1000MP1, 230);
    lv_obj_set_height(ui_setupYaesuFT1000MP1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT1000MP1, 245);
    lv_obj_set_y(ui_setupYaesuFT1000MP1, 137);
    lv_label_set_text(ui_setupYaesuFT1000MP1, "FT1000 MP1");
    lv_obj_set_style_text_font(ui_setupYaesuFT1000MP1, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT1000MP1, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT1000MP1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT1000MP1, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT1000MP1, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuFT1000MP2 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT1000MP2, 230);
    lv_obj_set_height(ui_setupYaesuFT1000MP2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT1000MP2, 530);
    lv_obj_set_y(ui_setupYaesuFT1000MP2, 25);
    lv_label_set_text(ui_setupYaesuFT1000MP2, "FT1000 MP2");
    lv_obj_set_style_text_font(ui_setupYaesuFT1000MP2, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT1000MP2, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT1000MP2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT1000MP2, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT1000MP2, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuFT1000MP3 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT1000MP3, 230);
    lv_obj_set_height(ui_setupYaesuFT1000MP3, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT1000MP3, 530);
    lv_obj_set_y(ui_setupYaesuFT1000MP3, 53);
    lv_label_set_text(ui_setupYaesuFT1000MP3, "FT1000 MP3");
    lv_obj_set_style_text_font(ui_setupYaesuFT1000MP3, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT1000MP3, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT1000MP3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT1000MP3, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT1000MP3, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuFT2000 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT2000, 230);
    lv_obj_set_height(ui_setupYaesuFT2000, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT2000, 530);
    lv_obj_set_y(ui_setupYaesuFT2000, 81);
    lv_label_set_text(ui_setupYaesuFT2000, "FT2000");
    lv_obj_set_style_text_font(ui_setupYaesuFT2000, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT2000, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT2000, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT2000, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT2000, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuFT9000 = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuFT9000, 230);
    lv_obj_set_height(ui_setupYaesuFT9000, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuFT9000, 530);
    lv_obj_set_y(ui_setupYaesuFT9000, 109);
    lv_label_set_text(ui_setupYaesuFT9000, "FT9000D");
    lv_obj_set_style_text_font(ui_setupYaesuFT9000, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuFT9000, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuFT9000, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuFT9000, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuFT9000, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupYaesuBandData = lv_label_create(ui_setupYaesuOptions);
    lv_obj_set_width(ui_setupYaesuBandData, 230);
    lv_obj_set_height(ui_setupYaesuBandData, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupYaesuBandData, 530);
    lv_obj_set_y(ui_setupYaesuBandData, 137);
    lv_label_set_text(ui_setupYaesuBandData, "BAND_DATA");
    lv_obj_set_style_text_font(ui_setupYaesuBandData, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupYaesuBandData, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupYaesuBandData, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupYaesuBandData, -4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupYaesuBandData, -2, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Icom Options
    ui_setupIcomCiv = lv_label_create(ui_setupIcomOptions);
    lv_obj_set_width(ui_setupIcomCiv, 270);
    lv_obj_set_height(ui_setupIcomCiv, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupIcomCiv, 245);
    lv_obj_set_y(ui_setupIcomCiv, 45);
    lv_label_set_text(ui_setupIcomCiv, "CI-V");
    lv_obj_set_style_text_font(ui_setupIcomCiv, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupIcomCiv, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupIcomCiv, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupIcomCiv, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupIcomCiv, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupIcomVoltage = lv_label_create(ui_setupIcomOptions);
    lv_obj_set_width(ui_setupIcomVoltage, 270);
    lv_obj_set_height(ui_setupIcomVoltage, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupIcomVoltage, 245);
    lv_obj_set_y(ui_setupIcomVoltage, 75);
    lv_label_set_text(ui_setupIcomVoltage, "VOLTAGE_BAND");
    lv_obj_set_style_text_font(ui_setupIcomVoltage, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupIcomVoltage, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupIcomVoltage, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupIcomVoltage, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupIcomVoltage, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // TenTec
    ui_setupTenTecOmni = lv_label_create(ui_setupTenTecOptions);
    lv_obj_set_width(ui_setupTenTecOmni, 235);
    lv_obj_set_height(ui_setupTenTecOmni, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupTenTecOmni, 135);
    lv_obj_set_y(ui_setupTenTecOmni, 60);
    lv_label_set_text(ui_setupTenTecOmni, "OMNI VII");
    lv_obj_set_style_text_font(ui_setupTenTecOmni, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupTenTecOmni, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupTenTecOmni, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupTenTecOmni, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupTenTecOmni, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupTenTecOrion = lv_label_create(ui_setupTenTecOptions);
    lv_obj_set_width(ui_setupTenTecOrion, 235);
    lv_obj_set_height(ui_setupTenTecOrion, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupTenTecOrion, 135);
    lv_obj_set_y(ui_setupTenTecOrion, 90);
    lv_label_set_text(ui_setupTenTecOrion, "ORION I/II");
    lv_obj_set_style_text_font(ui_setupTenTecOrion, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupTenTecOrion, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupTenTecOrion, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupTenTecOrion, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupTenTecOrion, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupTenTecJupiter = lv_label_create(ui_setupTenTecOptions);
    lv_obj_set_width(ui_setupTenTecJupiter, 235);
    lv_obj_set_height(ui_setupTenTecJupiter, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupTenTecJupiter, 405);
    lv_obj_set_y(ui_setupTenTecJupiter, 60);
    lv_label_set_text(ui_setupTenTecJupiter, "JUPITER");
    lv_obj_set_style_text_font(ui_setupTenTecJupiter, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupTenTecJupiter, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupTenTecJupiter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupTenTecJupiter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupTenTecJupiter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupTenTecArgonaut = lv_label_create(ui_setupTenTecOptions);
    lv_obj_set_width(ui_setupTenTecArgonaut, 235);
    lv_obj_set_height(ui_setupTenTecArgonaut, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupTenTecArgonaut, 405);
    lv_obj_set_y(ui_setupTenTecArgonaut, 90);
    lv_label_set_text(ui_setupTenTecArgonaut, "ARGONAUT V");
    lv_obj_set_style_text_font(ui_setupTenTecArgonaut, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupTenTecArgonaut, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupTenTecArgonaut, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupTenTecArgonaut, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupTenTecArgonaut, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Baud Rate
    ui_setupBaud1200 = lv_label_create(ui_setupBaudRateContainer2);
    lv_obj_set_width(ui_setupBaud1200, 105);
    lv_obj_set_height(ui_setupBaud1200, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupBaud1200, 250);
    lv_obj_set_y(ui_setupBaud1200, 0);
    lv_label_set_text(ui_setupBaud1200, "1200");
    lv_obj_set_style_text_font(ui_setupBaud1200, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupBaud1200, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupBaud1200, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupBaud1200, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupBaud1200, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupBaud2400 = lv_label_create(ui_setupBaudRateContainer2);
    lv_obj_set_width(ui_setupBaud2400, 105);
    lv_obj_set_height(ui_setupBaud2400, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupBaud2400, 250);
    lv_obj_set_y(ui_setupBaud2400, 30);
    lv_label_set_text(ui_setupBaud2400, "2400");
    lv_obj_set_style_text_font(ui_setupBaud2400, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupBaud2400, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupBaud2400, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupBaud2400, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupBaud2400, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupBaud4800 = lv_label_create(ui_setupBaudRateContainer2);
    lv_obj_set_width(ui_setupBaud4800, 105);
    lv_obj_set_height(ui_setupBaud4800, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupBaud4800, 250);
    lv_obj_set_y(ui_setupBaud4800, 60);
    lv_label_set_text(ui_setupBaud4800, "4800");
    lv_obj_set_style_text_font(ui_setupBaud4800, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupBaud4800, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupBaud4800, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupBaud4800, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupBaud4800, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_setupBaud9600 = lv_label_create(ui_setupBaudRateContainer2);
    lv_obj_set_width(ui_setupBaud9600, 105);
    lv_obj_set_height(ui_setupBaud9600, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_setupBaud9600, 250);
    lv_obj_set_y(ui_setupBaud9600, 90);
    lv_label_set_text(ui_setupBaud9600, "9600");
    lv_obj_set_style_text_font(ui_setupBaud9600, &ui_font_LCD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_setupBaud9600, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_setupBaud9600, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_setupBaud9600, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_setupBaud9600, 0, LV_PART_MAIN | LV_STATE_DEFAULT);


}