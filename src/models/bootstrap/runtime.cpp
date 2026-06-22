/*
 * Generic amplifier bootstrap runtime used before protocol detection.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/bootstrap/runtime.h"

#include "amp_dtr.h"
#include "models/spe_expert1k/protocol.h"
#include "models/spe_expert1k/serial_link.h"

AmpModelId BootstrapRuntime::model_id() const
{
  return AmpModelId::Unknown;
}

bool BootstrapRuntime::status_valid() const
{
  return false;
}

uint8_t BootstrapRuntime::screen_id() const
{
  return 0;
}

const char *BootstrapRuntime::screen_name() const
{
  return "detecting";
}

unsigned long BootstrapRuntime::last_activity_ms() const
{
  return last_probe_ms_;
}

void BootstrapRuntime::publish_status() const
{
}

bool BootstrapRuntime::press_key(const char *name)
{
  (void)name;
  return false;
}

bool BootstrapRuntime::power_on()
{
  amp_dtr_set(true);
  return spe_expert1k_queue_command({Rcu_On});
}

bool BootstrapRuntime::refresh_remote()
{
  return power_on();
}

bool BootstrapRuntime::remote_updates_enabled() const
{
  return true;
}

void BootstrapRuntime::process_next_queued_command()
{
  spe_expert1k_process_next_queued_command();
}

void BootstrapRuntime::mark_activity(unsigned long now)
{
  last_probe_ms_ = now;
}

bool BootstrapRuntime::should_send_keepalive(unsigned long now, unsigned long interval_ms) const
{
  return now - last_probe_ms_ >= interval_ms;
}

void BootstrapRuntime::note_keepalive(unsigned long now)
{
  last_probe_ms_ = now;
}
