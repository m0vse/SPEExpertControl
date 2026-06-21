/*
 * SPE Expert 1K-FA adapter from raw protocol packets to model-neutral amplifier status.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include "models/amplifier_model.h"
#include "models/spe_expert1k/expertpackets.h"
#include "models/spe_expert1k/protocol.h"

AmpStatusSnapshot spe_expert1k_make_status_snapshot(bool valid, ExpertScreen screen, const Expert_Packet &packet);
