/*
 * LVGL custom SPE component implementation.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */


#include "ui.h"

// COMPONENT spe

lv_obj_t * ui_spe_create(lv_obj_t * comp_parent)
{

    lv_obj_t * cui_spe;
    cui_spe = lv_image_create(comp_parent);
    lv_obj_set_width(cui_spe, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(cui_spe, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(cui_spe, -309);
    lv_obj_set_y(cui_spe, -187);
    lv_obj_set_align(cui_spe, LV_ALIGN_CENTER);
    lv_obj_add_flag(cui_spe, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_remove_flag(cui_spe, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    lv_obj_t ** children = static_cast<lv_obj_t **>(lv_malloc(sizeof(lv_obj_t *) * _UI_COMP_SPE_NUM));
    children[UI_COMP_SPE_SPE] = cui_spe;
    lv_obj_add_event_cb(cui_spe, get_component_child_event_cb, static_cast<lv_event_code_t>(LV_EVENT_GET_COMP_CHILD), children);
    lv_obj_add_event_cb(cui_spe, del_component_child_event_cb, LV_EVENT_DELETE, children);
    ui_comp_spe_create_hook(cui_spe);
    return cui_spe;
}

