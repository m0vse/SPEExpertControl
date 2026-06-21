/*
 * SPE Expert 1K-FA LCD rendering helpers for raw protocol screen data.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include "models/amplifier_model.h"
#include "models/spe_expert1k/expertpackets.h"

void spe_expert1k_configure_transmit_meters(const AmpStatusSnapshot &status);
void spe_expert1k_update_cat_screen(const Expert_Packet &packet);
