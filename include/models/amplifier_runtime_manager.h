/*
 * Active amplifier runtime registry and selection helpers.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include "models/amplifier_model.h"

class AmplifierRuntime;
class SpeExpert1kRuntime;

/**
 * Select the active runtime implementation for the configured amplifier model.
 *
 * Only the SPE Expert 1K-FA runtime is currently implemented. Unsupported
 * model IDs are rejected by returning false and leaving the current runtime
 * unchanged.
 */
bool amplifier_runtime_select(AmpModelId model);

/**
 * Return the currently selected runtime. A valid runtime is available after
 * amplifier_runtime_select() has been called during setup.
 */
AmplifierRuntime *amplifier_runtime_active();

/**
 * Return the active model ID, or Unknown if no runtime has been selected yet.
 */
AmpModelId amplifier_runtime_active_model_id();

/**
 * Return the concrete SPE Expert 1K-FA runtime when it is active.
 *
 * This is a temporary adapter while the 1K packet/UI path is being lifted
 * behind the generic runtime interface.
 */
SpeExpert1kRuntime *amplifier_runtime_spe_expert1k();
