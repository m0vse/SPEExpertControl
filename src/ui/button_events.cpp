/*
 * LVGL front-panel button event dispatch for amplifier control keys.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "ui_events.h"

#include "amp_control.h"
#include "ui_mainScreen.h"

void button_pressed(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_current_target_obj(e);
  if(code == LV_EVENT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
    if (obj == ui_buttonLowerL)
      amp_control_press_key("l_down");
    else if (obj == ui_buttonHigherL)
      amp_control_press_key("l_up");
    else if (obj == ui_buttonLowerC)
      amp_control_press_key("c_down");
    else if (obj == ui_buttonHigherC)
      amp_control_press_key("c_up");
    else if (obj == ui_buttonTune)
      amp_control_press_key("tune");
    else if (obj == ui_buttonInput)
      amp_control_press_key("input");
    if (obj == ui_buttonBandDown)
      amp_control_press_key("band_down");
    else if (obj == ui_buttonBandUp)
      amp_control_press_key("band_up");
    if (obj == ui_buttonAnt)
      amp_control_press_key("ant");
    if (obj == ui_buttonLeftUp)
      amp_control_press_key("left");
    else if (obj == ui_buttonRightDown)
      amp_control_press_key("right");
    else if (obj == ui_buttonCat)
      amp_control_press_key("cat");
    else if (obj == ui_buttonSet)
      amp_control_press_key("set");
    else if (obj == ui_buttonOff)
      amp_control_press_key("off");
    else if (obj == ui_buttonOn)
      amp_control_power_on();
    else if (obj == ui_buttonPower)
      amp_control_press_key("power");
    else if (obj == ui_buttonDisplay)
      amp_control_press_key("display");
    else if (obj == ui_buttonOperate)
      amp_control_press_key("operate");
  }
}
