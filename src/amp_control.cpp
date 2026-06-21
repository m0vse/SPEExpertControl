/*
 * Public amplifier control API and SPE Expert 1K-FA key-command mapping.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "amp_control.h"

#include "amp_dtr.h"
#include "models/spe_expert1k/protocol.h"
#include "models/spe_expert1k/serial_link.h"
#include <initializer_list>
#include <string.h>

static bool remote_update_enabled = true;

static bool send_command(std::initializer_list<uint8_t> cmd)
{
  const bool rcu_on = cmd.size() == 1 && *cmd.begin() == Rcu_On;
  const bool off_key = cmd.size() == 2 && *cmd.begin() == Key_On && *(cmd.begin() + 1) == Off_Key;

  if (rcu_on) {
    remote_update_enabled = true;
    amp_dtr_set(true);
  }

  if (off_key) {
    amp_dtr_set(false);
    if (!spe_expert1k_queue_command(cmd)) {
      return false;
    }
    if (!spe_expert1k_queue_command({Rcu_Off})) {
      return false;
    }
    remote_update_enabled = false;
    return true;
  }

  return spe_expert1k_queue_command(cmd);
}

bool amp_control_power_on()
{
  return send_command({Rcu_On});
}

bool amp_control_remote_updates_enabled()
{
  return remote_update_enabled;
}

bool amp_control_press_key(const char *name)
{
  if (strcmp(name, "l_down") == 0) {
    return send_command({Key_On, L_Minus_Key});
  } else if (strcmp(name, "l_up") == 0) {
    return send_command({Key_On, L_Plus_Key});
  } else if (strcmp(name, "c_down") == 0) {
    return send_command({Key_On, C_Minus_Key});
  } else if (strcmp(name, "c_up") == 0) {
    return send_command({Key_On, C_Plus_Key});
  } else if (strcmp(name, "tune") == 0) {
    return send_command({Key_On, Tune_Key});
  } else if (strcmp(name, "input") == 0) {
    return send_command({Key_On, In_Key});
  } else if (strcmp(name, "band_down") == 0) {
    return send_command({Key_On, Band_Minus_Key});
  } else if (strcmp(name, "band_up") == 0) {
    return send_command({Key_On, Band_Plus_Key});
  } else if (strcmp(name, "ant") == 0) {
    return send_command({Key_On, Ant_Key});
  } else if (strcmp(name, "left") == 0) {
    return send_command({Key_On, Left_Key});
  } else if (strcmp(name, "right") == 0) {
    return send_command({Key_On, Right_Key});
  } else if (strcmp(name, "cat") == 0) {
    return send_command({Key_On, Cat_Key});
  } else if (strcmp(name, "set") == 0) {
    return send_command({Key_On, Set_Key});
  } else if (strcmp(name, "off") == 0) {
    return send_command({Key_On, Off_Key});
  } else if (strcmp(name, "on") == 0) {
    return amp_control_power_on();
  } else if (strcmp(name, "power") == 0) {
    return send_command({Key_On, Power_Key});
  } else if (strcmp(name, "display") == 0) {
    return send_command({Key_On, Display_Key});
  } else if (strcmp(name, "operate") == 0) {
    return send_command({Key_On, Operate_Key});
  }

  return false;
}
