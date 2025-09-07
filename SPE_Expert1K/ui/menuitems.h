#ifndef MENUITEMS_H
#define MENUITEMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

extern void setupMenus(void);

// Main setup options menu
extern lv_obj_t * ui_setupAntenna;
extern lv_obj_t * ui_setupContest;
extern lv_obj_t * ui_setupBeep;
extern lv_obj_t * ui_setupStart;
extern lv_obj_t * ui_setupTemp;
extern lv_obj_t * ui_setupQuit;
extern lv_obj_t * ui_setupCat;
extern lv_obj_t * ui_setupManualTune;
extern lv_obj_t * ui_setupBacklight;

// Setup antenna menu
// Container1
extern lv_obj_t * ui_setup160AntLabel;
extern lv_obj_t * ui_setup30AntLabel;
extern lv_obj_t * ui_setup40AntLabel;
extern lv_obj_t * ui_setup80AntLabel;
extern lv_obj_t * ui_setup160Ant1;
extern lv_obj_t * ui_setup160Ant2;
extern lv_obj_t * ui_setup80Ant1;
extern lv_obj_t * ui_setup80Ant2;
extern lv_obj_t * ui_setup40Ant1;
extern lv_obj_t * ui_setup40Ant2;
extern lv_obj_t * ui_setup30Ant1;
extern lv_obj_t * ui_setup30Ant2;
// Container2
extern lv_obj_t * ui_setup20AntLabel;
extern lv_obj_t * ui_setup12AntLabel;
extern lv_obj_t * ui_setup15AntLabel;
extern lv_obj_t * ui_setup17AntLabel;
extern lv_obj_t * ui_setup20Ant1;
extern lv_obj_t * ui_setup20Ant2;
extern lv_obj_t * ui_setup17Ant1;
extern lv_obj_t * ui_setup17Ant2;
extern lv_obj_t * ui_setup15Ant1;
extern lv_obj_t * ui_setup15Ant2;
extern lv_obj_t * ui_setup12Ant1;
extern lv_obj_t * ui_setup12Ant2;
// Container3
extern lv_obj_t * ui_setup10AntLabel;
extern lv_obj_t * ui_setup6AntLabel;
extern lv_obj_t * ui_setup10Ant1;
extern lv_obj_t * ui_setup10Ant2;
extern lv_obj_t * ui_setup6Ant1;
extern lv_obj_t * ui_setup6Ant2;
extern lv_obj_t * ui_setupAntSave;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
