/*
 * SPE Expert 1K-FA protocol constants and screen-name helpers.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <stdint.h>

enum ExpertCommands {
  Key_On = 0x10,
  Rcu_On = 0x80,
  Rcu_Off = 0x81,
  Cat_232 = 0x82
};

enum ExpertKeys {
  L_Minus_Key = 0x30,
  L_Plus_Key = 0x31,
  C_Minus_Key = 0x32,
  C_Plus_Key = 0x33,
  Tune_Key = 0x34,
  In_Key = 0x28,
  Band_Minus_Key = 0x29,
  Band_Plus_Key = 0x2A,
  Ant_Key = 0x2B,
  Cat_Key = 0x2C,
  Left_Key = 0x2D,
  Right_Key = 0x2E,
  Set_Key = 0x2F,
  Off_Key = 0x18,
  Power_Key = 0x1A,
  Display_Key = 0x1B,
  Operate_Key = 0x1C
};

enum ExpertScreen {
  Receive_Screen = 0x00,
  Operate_RX,
  Operate_TX,
  Cat_Screen,
  Unused_Screen_A,
  Data_Stored,
  Setup_Options,
  Set_Antenna,
  Set_Cat,
  Set_Yaesu,
  Set_Icom,
  Set_TenTec,
  Set_BaudRate,
  Manual_Tune,
  Backlight,
  Unused_Screen_B,
  Unused_Screen_C,
  Warning_V_Low_Half,
  Warning_V_Low_Full,
  Warning_V_High_Half,
  Warning_V_High_Full,
  Warning_A_High_Half,
  Warning_A_High_Full,
  Warning_Temp,
  Warning_Over_Driving,
  Unused_Screen_D,
  Unused_Screen_E,
  Warning_Reverse,
  Warning_Protection,
  Alarm_History = 0x1D,
  Shutdown = 0x1E,
  BootMessage = 0xff
};

const char *screen_name(ExpertScreen value);
