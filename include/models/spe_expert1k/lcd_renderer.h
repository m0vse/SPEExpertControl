/*
 * SPE Expert 1K-FA LCD rendering helpers for raw protocol screen data.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <stddef.h>
#include <lvgl.h>

#include "models/amplifier_model.h"
#include "models/spe_expert1k/expertpackets.h"

void spe_expert1k_configure_transmit_meters(const AmpStatusSnapshot &status);
void spe_expert1k_update_cat_screen(const Expert_Packet &packet);
void spe_expert1k_show_antenna_setup_screen(const Expert_Packet &packet,
                                            lv_obj_t * const antenna_items[],
                                            size_t antenna_item_count);
uint8_t spe_expert1k_update_antenna_setup_screen(const Expert_Packet &packet,
                                                 lv_obj_t * const antenna_items[],
                                                 size_t antenna_item_count,
                                                 uint8_t previous_selection);
void spe_expert1k_update_manual_tune_screen(const Expert_Packet &packet);
