/*
 * Active amplifier runtime registry and selection helpers.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/amplifier_runtime_manager.h"

#include "models/amplifier_runtime.h"
#include "models/spe_expert1k/runtime.h"
#include "models/spe_expert1k/session.h"

static SpeExpert1kRuntime spe_expert1k_runtime;
static SpeExpert1kSerialSession spe_expert1k_session;
static AmplifierRuntime *active_runtime = nullptr;
static AmplifierSerialSession *active_session = nullptr;

bool amplifier_runtime_select(AmpModelId model)
{
  switch (model) {
    case AmpModelId::SpeExpert1k:
      active_runtime = &spe_expert1k_runtime;
      active_session = &spe_expert1k_session;
      return true;
    default:
      return false;
  }
}

AmplifierRuntime *amplifier_runtime_active()
{
  return active_runtime;
}

AmplifierSerialSession *amplifier_session_active()
{
  return active_session;
}

AmpModelId amplifier_runtime_active_model_id()
{
  return active_runtime ? active_runtime->model_id() : AmpModelId::Unknown;
}

SpeExpert1kRuntime *amplifier_runtime_spe_expert1k()
{
  if (active_runtime && active_runtime->model_id() == AmpModelId::SpeExpert1k) {
    return &spe_expert1k_runtime;
  }
  return nullptr;
}
