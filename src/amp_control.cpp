/*
 * Public amplifier control facade delegated to the active amplifier runtime.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "amp_control.h"

#include "models/amplifier_runtime.h"

static AmplifierRuntime *active_runtime = nullptr;

void amp_control_bind_runtime(AmplifierRuntime *runtime)
{
  active_runtime = runtime;
}

bool amp_control_power_on()
{
  return active_runtime ? active_runtime->power_on() : false;
}

bool amp_control_remote_updates_enabled()
{
  return active_runtime ? active_runtime->remote_updates_enabled() : false;
}

bool amp_control_press_key(const char *name)
{
  return active_runtime ? active_runtime->press_key(name) : false;
}

void amp_control_process_next_queued_command()
{
  if (active_runtime) {
    active_runtime->process_next_queued_command();
  }
}
