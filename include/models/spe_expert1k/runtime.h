/*
 * SPE Expert 1K-FA runtime state and packet lifecycle handling.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <initializer_list>
#include <stddef.h>
#include <lvgl.h>

#include "models/amplifier_runtime.h"
#include "models/spe_expert1k/expertpackets.h"
#include "models/spe_expert1k/protocol.h"
#include "ui/menu_control.h"

struct SpeExpert1kRuntimeBindings {
  MenuController *setup_options = nullptr;
  MenuController *setup_ant = nullptr;
  MenuController *setup_cat = nullptr;
  MenuController *setup_yaesu = nullptr;
  MenuController *setup_icom = nullptr;
  MenuController *setup_tentec = nullptr;
  MenuController *setup_baudrate = nullptr;
  lv_obj_t * const *setup_ant_items = nullptr;
  size_t setup_ant_item_count = 0;
};

class SpeExpert1kRuntime : public AmplifierRuntime {
public:
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

  ExpertScreen screen() const;
  const Expert_Packet &last_status() const;
  void mark_activity(unsigned long now);
  bool should_send_keepalive(unsigned long now, unsigned long interval_ms) const;
  void note_keepalive(unsigned long now);
  void process_status_packet(const Expert_Packet &packet,
                             const SpeExpert1kRuntimeBindings &bindings,
                             unsigned long now);

private:
  bool send_command(std::initializer_list<uint8_t> cmd);

  bool status_valid_ = false;
  bool remote_update_enabled_ = true;
  ExpertScreen screen_ = BootMessage;
  Expert_Packet last_status_{};
  Expert_Packet web_cat_snapshot_{};
  unsigned long web_cat_snapshot_until_ = 0;
  unsigned long last_activity_ms_ = 0;
  static constexpr unsigned long cat_display_hold_ms_ = 6000;
};
