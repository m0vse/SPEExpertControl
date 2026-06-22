/*
 * LVGL front-panel button event dispatch for amplifier control keys.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "ui_events.h"

#include "amp_control.h"
#include "ui_mainScreen.h"

static void set_button_active(lv_obj_t *obj, bool active)
{
  if (!obj) {
    return;
  }
  lv_obj_set_style_bg_color(obj, lv_color_hex(active ? 0xFF5F00 : 0x5A5A5A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(obj, lv_color_hex(active ? 0x111111 : 0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_invalidate(obj);
}

static bool dispatch_button(lv_obj_t *obj)
{
  if (obj == ui_buttonLowerL)
    return amp_control_press_key("l_down");
  else if (obj == ui_buttonHigherL)
    return amp_control_press_key("l_up");
  else if (obj == ui_buttonLowerC)
    return amp_control_press_key("c_down");
  else if (obj == ui_buttonHigherC)
    return amp_control_press_key("c_up");
  else if (obj == ui_buttonTune)
    return amp_control_press_key("tune");
  else if (obj == ui_buttonInput)
    return amp_control_press_key("input");
  else if (obj == ui_buttonBandDown)
    return amp_control_press_key("band_down");
  else if (obj == ui_buttonBandUp)
    return amp_control_press_key("band_up");
  else if (obj == ui_buttonAnt)
    return amp_control_press_key("ant");
  else if (obj == ui_buttonLeftUp)
    return amp_control_press_key("left");
  else if (obj == ui_buttonRightDown)
    return amp_control_press_key("right");
  else if (obj == ui_buttonCat)
    return amp_control_press_key("cat");
  else if (obj == ui_buttonSet)
    return amp_control_press_key("set");
  else if (obj == ui_buttonOff)
    return amp_control_press_key("off");
  else if (obj == ui_buttonOn)
    return amp_control_power_on();
  else if (obj == ui_buttonPower)
    return amp_control_press_key("power");
  else if (obj == ui_buttonDisplay)
    return amp_control_press_key("display");
  else if (obj == ui_buttonOperate)
    return amp_control_press_key("operate");
  return false;
}

void button_pressed(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_current_target_obj(e);

  if (code == LV_EVENT_PRESSED) {
    set_button_active(obj, true);
  } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
    set_button_active(obj, false);
  } else if (code == LV_EVENT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
    dispatch_button(obj);
  }
}
