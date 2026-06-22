/*
 * Placeholder runtime for newer SPE Expert amplifier families.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include "models/amplifier_runtime.h"

class SpeModernRuntime : public AmplifierRuntime {
public:
  explicit SpeModernRuntime(AmpModelId model_id = AmpModelId::SpeExpert13k);

  void set_model_id(AmpModelId model_id);
  AmpModelId model_id() const override;
  bool status_valid() const override;
  uint8_t screen_id() const override;
  const char *screen_name() const override;
  unsigned long last_activity_ms() const override;
  void publish_status() const override;
  bool press_key(const char *name) override;
  bool power_on() override;
  bool remote_updates_enabled() const override;
  void process_next_queued_command() override;
  void mark_activity(unsigned long now) override;
  bool should_send_keepalive(unsigned long now, unsigned long interval_ms) const override;
  void note_keepalive(unsigned long now) override;

private:
  AmpModelId model_id_ = AmpModelId::SpeExpert13k;
  unsigned long last_activity_ms_ = 0;
};
