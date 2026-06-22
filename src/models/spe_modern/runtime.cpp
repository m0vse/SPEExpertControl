/*
 * Placeholder runtime for newer SPE Expert amplifier families.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/spe_modern/runtime.h"

SpeModernRuntime::SpeModernRuntime(AmpModelId model_id)
  : model_id_(model_id)
{
}

void SpeModernRuntime::set_model_id(AmpModelId model_id)
{
  model_id_ = model_id;
}

AmpModelId SpeModernRuntime::model_id() const
{
  return model_id_;
}

bool SpeModernRuntime::status_valid() const
{
  return false;
}

uint8_t SpeModernRuntime::screen_id() const
{
  return 0;
}

const char *SpeModernRuntime::screen_name() const
{
  return "modern_stub";
}

unsigned long SpeModernRuntime::last_activity_ms() const
{
  return last_activity_ms_;
}

void SpeModernRuntime::publish_status() const
{
}

bool SpeModernRuntime::press_key(const char *name)
{
  (void)name;
  return false;
}

bool SpeModernRuntime::power_on()
{
  return false;
}

bool SpeModernRuntime::remote_updates_enabled() const
{
  return false;
}

void SpeModernRuntime::process_next_queued_command()
{
}

void SpeModernRuntime::mark_activity(unsigned long now)
{
  last_activity_ms_ = now;
}

bool SpeModernRuntime::should_send_keepalive(unsigned long now, unsigned long interval_ms) const
{
  (void)now;
  (void)interval_ms;
  return false;
}

void SpeModernRuntime::note_keepalive(unsigned long now)
{
  last_activity_ms_ = now;
}
