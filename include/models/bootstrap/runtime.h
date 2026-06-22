/*
 * Generic amplifier bootstrap runtime used before protocol detection.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include "models/amplifier_runtime.h"

class BootstrapRuntime : public AmplifierRuntime {
public:
  AmpModelId model_id() const override;
  bool status_valid() const override;
  uint8_t screen_id() const override;
  const char *screen_name() const override;
  unsigned long last_activity_ms() const override;
  void publish_status() const override;
  bool press_key(const char *name) override;
  bool power_on() override;
  bool refresh_remote() override;
  bool remote_updates_enabled() const override;
  void process_next_queued_command() override;
  void mark_activity(unsigned long now) override;
  bool should_send_keepalive(unsigned long now, unsigned long interval_ms) const override;
  void note_keepalive(unsigned long now) override;

private:
  unsigned long last_probe_ms_ = 0;
};
