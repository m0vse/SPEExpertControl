/*
 * SPE Expert 1K-FA runtime state and packet lifecycle handling.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/spe_expert1k/runtime.h"

#include <string.h>
#include <ui.h>

#include "models/spe_expert1k/lcd_renderer.h"
#include "models/spe_expert1k/serial_link.h"
#include "models/spe_expert1k/status_adapter.h"
#include "amp_dtr.h"

AmpModelId SpeExpert1kRuntime::model_id() const
{
  return AmpModelId::SpeExpert1k;
}

bool SpeExpert1kRuntime::status_valid() const
{
  return status_valid_;
}

uint8_t SpeExpert1kRuntime::screen_id() const
{
  return static_cast<uint8_t>(screen_);
}

const char *SpeExpert1kRuntime::screen_name() const
{
  return ::screen_name(screen_);
}

unsigned long SpeExpert1kRuntime::last_activity_ms() const
{
  return last_activity_ms_;
}

void SpeExpert1kRuntime::publish_status() const
{
  spe_expert1k_publish_app_status(status_valid_, screen_, last_status_, web_cat_snapshot_, web_cat_snapshot_until_);
}

bool SpeExpert1kRuntime::send_command(std::initializer_list<uint8_t> cmd)
{
  const bool rcu_on = cmd.size() == 1 && *cmd.begin() == Rcu_On;
  const bool off_key = cmd.size() == 2 && *cmd.begin() == Key_On && *(cmd.begin() + 1) == Off_Key;

  if (rcu_on) {
    remote_update_enabled_ = true;
    amp_dtr_set(true);
  }

  if (off_key) {
    amp_dtr_set(false);
    if (!spe_expert1k_queue_command(cmd)) {
      return false;
    }
    if (!spe_expert1k_queue_command({Rcu_Off})) {
      return false;
    }
    remote_update_enabled_ = false;
    return true;
  }

  return spe_expert1k_queue_command(cmd);
}

bool SpeExpert1kRuntime::press_key(const char *name)
{
  if (strcmp(name, "l_down") == 0) {
    return send_command({Key_On, L_Minus_Key});
  } else if (strcmp(name, "l_up") == 0) {
    return send_command({Key_On, L_Plus_Key});
  } else if (strcmp(name, "c_down") == 0) {
    return send_command({Key_On, C_Minus_Key});
  } else if (strcmp(name, "c_up") == 0) {
    return send_command({Key_On, C_Plus_Key});
  } else if (strcmp(name, "tune") == 0) {
    return send_command({Key_On, Tune_Key});
  } else if (strcmp(name, "input") == 0) {
    return send_command({Key_On, In_Key});
  } else if (strcmp(name, "band_down") == 0) {
    return send_command({Key_On, Band_Minus_Key});
  } else if (strcmp(name, "band_up") == 0) {
    return send_command({Key_On, Band_Plus_Key});
  } else if (strcmp(name, "ant") == 0) {
    return send_command({Key_On, Ant_Key});
  } else if (strcmp(name, "left") == 0) {
    return send_command({Key_On, Left_Key});
  } else if (strcmp(name, "right") == 0) {
    return send_command({Key_On, Right_Key});
  } else if (strcmp(name, "cat") == 0) {
    return send_command({Key_On, Cat_Key});
  } else if (strcmp(name, "set") == 0) {
    return send_command({Key_On, Set_Key});
  } else if (strcmp(name, "off") == 0) {
    return send_command({Key_On, Off_Key});
  } else if (strcmp(name, "on") == 0) {
    return power_on();
  } else if (strcmp(name, "power") == 0) {
    return send_command({Key_On, Power_Key});
  } else if (strcmp(name, "display") == 0) {
    return send_command({Key_On, Display_Key});
  } else if (strcmp(name, "operate") == 0) {
    return send_command({Key_On, Operate_Key});
  }

  return false;
}

bool SpeExpert1kRuntime::power_on()
{
  return send_command({Rcu_On});
}

bool SpeExpert1kRuntime::refresh_remote()
{
  return power_on();
}

bool SpeExpert1kRuntime::remote_updates_enabled() const
{
  return remote_update_enabled_;
}

void SpeExpert1kRuntime::process_next_queued_command()
{
  spe_expert1k_process_next_queued_command();
}

ExpertScreen SpeExpert1kRuntime::screen() const
{
  return screen_;
}

const Expert_Packet &SpeExpert1kRuntime::last_status() const
{
  return last_status_;
}

void SpeExpert1kRuntime::mark_activity(unsigned long now)
{
  last_activity_ms_ = now;
}

bool SpeExpert1kRuntime::should_send_keepalive(unsigned long now, unsigned long interval_ms) const
{
  return now - last_activity_ms_ >= interval_ms;
}

void SpeExpert1kRuntime::note_keepalive(unsigned long now)
{
  last_activity_ms_ = now;
}

void SpeExpert1kRuntime::process_status_packet(const Expert_Packet &packet,
                                               const SpeExpert1kRuntimeBindings &bindings,
                                               unsigned long now)
{
  status_valid_ = true;
  last_activity_ms_ = now;

  const ExpertScreen packet_screen = static_cast<ExpertScreen>(packet.display_ctx);
  AmpStatusSnapshot packet_status = spe_expert1k_make_status_snapshot(true, packet_screen, packet);
  const bool cat_hold_expired = screen_ == Cat_Screen && packet_screen != Cat_Screen && now >= web_cat_snapshot_until_;

  if (!memcmp(&last_status_, &packet, sizeof last_status_) && !cat_hold_expired) {
    return;
  }

  const bool holding_cat_screen = screen_ == Cat_Screen && packet_screen != Cat_Screen && now < web_cat_snapshot_until_;

  if (!holding_cat_screen && (screen_ != packet_screen || packet.flags != last_status_.flags || cat_hold_expired)) {
    if (packet_screen == Cat_Screen) {
      web_cat_snapshot_ = packet;
      web_cat_snapshot_until_ = now + cat_display_hold_ms_;
    }

    spe_expert1k_hide_status_screens();
    spe_expert1k_show_screen(packet_screen,
                             packet,
                             packet_status,
                             bindings.setup_ant_items,
                             bindings.setup_ant_item_count);
    if (packet_screen == Alarm_History) {
      last_status_.setup[0] = 0xff;
    }
    screen_ = packet_screen;
    if (packet_screen != Cat_Screen) {
      web_cat_snapshot_until_ = 0;
    }
    memset(&last_status_, 0xff, sizeof last_status_);
  }

  if (packet_screen == Alarm_History && last_status_.setup[0] != packet.setup[0]) {
    spe_expert1k_update_alarm_history_screen(packet);
  }

  if (memcmp(&last_status_.setup, &packet.setup, sizeof last_status_.setup) || last_status_.flags != packet.flags) {
    if (packet_screen == Setup_Options) {
      spe_expert1k_update_setup_options_screen(packet);
      if (bindings.setup_options) {
        bindings.setup_options->applySelection(packet.setup[1] & 0x0f);
      }
    } else if (packet_screen == Set_Antenna) {
      const uint8_t previous_selection = bindings.setup_ant ? bindings.setup_ant->selected() : 0;
      const uint8_t index = spe_expert1k_update_antenna_setup_screen(
        packet, bindings.setup_ant_items, bindings.setup_ant_item_count, previous_selection);
      if (bindings.setup_ant) {
        bindings.setup_ant->applySelection(index);
      }
    } else if (packet_screen == Set_Cat) {
      if (bindings.setup_cat) {
        bindings.setup_cat->applySelection(packet.setup[1]);
      }
    } else if (packet_screen == Set_Yaesu) {
      if (bindings.setup_yaesu) {
        bindings.setup_yaesu->applySelection(packet.setup[1]);
      }
    } else if (packet_screen == Set_Icom) {
      if (bindings.setup_icom) {
        bindings.setup_icom->applySelection(packet.setup[1]);
      }
    } else if (packet_screen == Set_TenTec) {
      if (bindings.setup_tentec) {
        bindings.setup_tentec->applySelection(packet.setup[1]);
      }
    } else if (packet_screen == Set_BaudRate) {
      if (bindings.setup_baudrate) {
        bindings.setup_baudrate->applySelection(packet.setup[1]);
      }
    } else if (packet_screen == Manual_Tune) {
      spe_expert1k_update_manual_tune_screen(packet);
    } else if (packet_screen == Backlight) {
      lv_bar_set_value(ui_backlightLevel, packet.setup[1], LV_ANIM_ON);
    }
  }

  if (packet_screen == Operate_RX || packet_screen == Operate_TX ||
      (((packet.flags >> 2) & 0x01) != 0 && packet_screen == Receive_Screen)) {
    spe_expert1k_configure_transmit_meters(packet_status);
  }

  spe_expert1k_update_status_values(last_status_, packet, packet_status);
  memcpy(&last_status_, &packet, sizeof last_status_);
  publish_status();
}
