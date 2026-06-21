/*
 * SPE Expert 1K-FA screen-name mapping used by LCD, web, and console status.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/spe_expert1k/protocol.h"

const char *screen_name(ExpertScreen value)
{
  switch (value) {
    case Receive_Screen: return "receive";
    case Operate_RX: return "operate_rx";
    case Operate_TX: return "operate_tx";
    case Cat_Screen: return "cat";
    case Data_Stored: return "data_stored";
    case Setup_Options: return "setup_options";
    case Set_Antenna: return "set_antenna";
    case Set_Cat: return "set_cat";
    case Set_Yaesu: return "set_yaesu";
    case Set_Icom: return "set_icom";
    case Set_TenTec: return "set_tentec";
    case Set_BaudRate: return "set_baudrate";
    case Manual_Tune: return "manual_tune";
    case Backlight: return "backlight";
    case Alarm_History: return "alarm_history";
    case Shutdown: return "shutdown";
    case BootMessage: return "boot";
    default: return "other";
  }
}
