/*
 * Active amplifier runtime registry and selection helpers.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/amplifier_runtime_manager.h"

#include "amp_control.h"
#include "models/amplifier_runtime.h"
#include "models/amplifier_session.h"
#include "models/bootstrap/runtime.h"
#include "models/bootstrap/session.h"
#include "models/spe_expert1k/runtime.h"
#include "models/spe_expert1k/session.h"
#include "models/spe_modern/runtime.h"
#include "models/spe_modern/session.h"

static BootstrapRuntime bootstrap_runtime;
static BootstrapSerialSession bootstrap_session;
static SpeExpert1kRuntime spe_expert1k_runtime;
static SpeExpert1kSerialSession spe_expert1k_session;
static SpeModernRuntime spe_modern_runtime;
static SpeModernSerialSession spe_modern_session;
static AmplifierRuntime *active_runtime = nullptr;
static AmplifierSerialSession *active_session = nullptr;
static AmpModelId detected_model = AmpModelId::Unknown;
static bool session_begin_on_select = false;

static bool select_runtime(AmpModelId model)
{
  switch (model) {
    case AmpModelId::Unknown:
      active_runtime = &bootstrap_runtime;
      active_session = &bootstrap_session;
      break;
    case AmpModelId::SpeExpert1k:
      active_runtime = &spe_expert1k_runtime;
      active_session = &spe_expert1k_session;
      break;
    case AmpModelId::SpeModern:
    case AmpModelId::SpeExpert13k:
    case AmpModelId::SpeExpert15k:
    case AmpModelId::SpeExpert2k:
      spe_modern_runtime.set_model_id(model);
      active_runtime = &spe_modern_runtime;
      active_session = &spe_modern_session;
      break;
    default:
      return false;
  }

  amp_control_bind_runtime(active_runtime);
  if (session_begin_on_select && active_session) {
    active_session->begin();
  }
  return true;
}

bool amplifier_runtime_select(AmpModelId model)
{
  return select_runtime(model);
}

bool amplifier_runtime_select_bootstrap_detector()
{
  return select_runtime(AmpModelId::Unknown);
}

void amplifier_runtime_begin_active_session()
{
  session_begin_on_select = true;
  if (active_session) {
    active_session->begin();
  }
}

void amplifier_runtime_note_detected_model(AmpModelId model)
{
  if (model == AmpModelId::Unknown || detected_model == model) {
    return;
  }

  detected_model = model;
  if (amp_model_available(model)) {
    amplifier_runtime_select(model);
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

AmpModelId amplifier_runtime_detected_model_id()
{
  return detected_model;
}

SpeExpert1kRuntime *amplifier_runtime_spe_expert1k()
{
  if (active_runtime && active_runtime->model_id() == AmpModelId::SpeExpert1k) {
    return &spe_expert1k_runtime;
  }
  return nullptr;
}
