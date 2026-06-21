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
#include "models/spe_expert1k/protocol.h"

void spe_expert1k_hide_status_screens();
void spe_expert1k_show_screen(ExpertScreen screen,
                              const Expert_Packet &packet,
                              const AmpStatusSnapshot &status,
                              lv_obj_t * const antenna_items[],
                              size_t antenna_item_count);
void spe_expert1k_configure_transmit_meters(const AmpStatusSnapshot &status);
void spe_expert1k_update_alarm_history_screen(const Expert_Packet &packet);
void spe_expert1k_update_setup_options_screen(const Expert_Packet &packet);
uint8_t spe_expert1k_update_antenna_setup_screen(const Expert_Packet &packet,
                                                 lv_obj_t * const antenna_items[],
                                                 size_t antenna_item_count,
                                                 uint8_t previous_selection);
void spe_expert1k_update_manual_tune_screen(const Expert_Packet &packet);
void spe_expert1k_update_status_values(const Expert_Packet &previous_packet,
                                       const Expert_Packet &packet,
                                       const AmpStatusSnapshot &status);
