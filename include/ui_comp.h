/*
 * LVGL component declaration header retained for generated UI compatibility.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */


#ifndef _UI_COMP__H
#define _UI_COMP__H

#include "ui.h"

void get_component_child_event_cb(lv_event_t * e);
void del_component_child_event_cb(lv_event_t * e);

lv_obj_t * ui_comp_get_child(lv_obj_t * comp, uint32_t child_idx);
extern uint32_t LV_EVENT_GET_COMP_CHILD;
#include "ui_comp_spe.h"

#endif
