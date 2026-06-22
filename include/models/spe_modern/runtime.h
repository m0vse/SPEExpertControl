/*
 * Runtime for newer SPE Expert amplifier families using the documented CSV protocol.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include "models/amplifier_runtime.h"

class SpeModernRuntime : public AmplifierRuntime {
public:
  explicit SpeModernRuntime(AmpModelId model_id = AmpModelId::SpeModern);

  void set_model_id(AmpModelId model_id);
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

  void process_status_packet(const uint8_t *data, uint8_t len, unsigned long now);
  void process_rcu_frame(const uint8_t *data, uint16_t len, unsigned long now);

private:
  uint32_t rcu_frame_signature(const uint8_t *data, uint16_t len) const;
  bool send_command(uint8_t opcode);
  bool send_key_command(uint8_t opcode);
  void publish_current_status() const;

  AmpModelId model_id_ = AmpModelId::SpeModern;
  unsigned long last_activity_ms_ = 0;
  unsigned long last_poll_ms_ = 0;
  bool status_valid_ = false;
  bool remote_update_enabled_ = false;
  uint8_t screen_id_ = 0;
  char screen_name_[16] = "modern";
  char input_[4] = "?";
  char band_[8] = "?";
  char antenna_[8] = "?";
  char cat_[8] = "SPE";
  char out_[8] = "?";
  char swr_[8] = "--.--";
  char temp_[12] = "?";
  char lcd_title_[41] = "";
  char lcd_body_[336] = "";
  char lcd_cells_[641] = "";
  char lcd_attrs_[81] = "";
  char lcd_footer_[41] = "";
  char lcd_hint_[100] = "";
  float power_ = 0.0f;
  float reverse_ = 0.0f;
  float voltage_ = 0.0f;
  float current_ = 0.0f;
  uint8_t op_status_ = 0;
  uint32_t last_rcu_frame_signature_ = 0;
};
