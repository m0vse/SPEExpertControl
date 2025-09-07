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
//Cat
extern lv_obj_t * ui_setupCatSpe;
extern lv_obj_t * ui_setupCatIcom;
extern lv_obj_t * ui_setupCatKenwood;
extern lv_obj_t * ui_setupCatYaesu;
extern lv_obj_t * ui_setupCatTenTec;
extern lv_obj_t * ui_setupCatFlexRadio;
extern lv_obj_t * ui_setupCatRs232;
extern lv_obj_t * ui_setupCatNone;
//Yaesu
extern lv_obj_t * ui_setupYaesuFT100;
extern lv_obj_t * ui_setupYaesuFT757;
extern lv_obj_t * ui_setupYaesuFT817;
extern lv_obj_t * ui_setupYaesuFT840;
extern lv_obj_t * ui_setupYaesuFT897;
extern lv_obj_t * ui_setupYaesuFT900;
extern lv_obj_t * ui_setupYaesuFT920;
extern lv_obj_t * ui_setupYaesuFT990;
extern lv_obj_t * ui_setupYaesuFT1000;
extern lv_obj_t * ui_setupYaesuFT1000MP1;
extern lv_obj_t * ui_setupYaesuFT1000MP2;
extern lv_obj_t * ui_setupYaesuFT1000MP3;
extern lv_obj_t * ui_setupYaesuFT2000;
extern lv_obj_t * ui_setupYaesuFT9000;
extern lv_obj_t * ui_setupYaesuBandData;
//Icom
extern lv_obj_t * ui_setupIcomCiv;
extern lv_obj_t * ui_setupIcomVoltage;
//TenTec
extern lv_obj_t * ui_setupTenTecOmni;
extern lv_obj_t * ui_setupTenTecOrion;
extern lv_obj_t * ui_setupTenTecJupiter;
extern lv_obj_t * ui_setupTenTecArgonaut;
// Baud Rate
extern lv_obj_t * ui_setupBaud1200;
extern lv_obj_t * ui_setupBaud2400;
extern lv_obj_t * ui_setupBaud4800;
extern lv_obj_t * ui_setupBaud9600;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
