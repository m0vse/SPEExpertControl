/*
 * Decoded SPE Expert 1K status packet view helpers.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef SPE_EXPERT1K_STATUS_VIEW_H
#define SPE_EXPERT1K_STATUS_VIEW_H

#include <stdint.h>

#include "models/spe_expert1k/expertpackets.h"

class SpeStatusView {
public:
  explicit SpeStatusView(const Expert_Packet &packet) : packet_(packet) {}

  uint8_t op_status() const { return packet_.display_ctx & 0x03; }
  bool full_power() const { return ((packet_.flags >> 4) & 0x01) != 0; }
  bool shows_output_current() const { return op_status() == 1; }

  const char *power_label() const { return shows_output_current() ? "PW OUT" : "PW REV"; }
  uint16_t power_raw_tenths() const { return shows_output_current() ? packet_.power : packet_.rev_power; }
  uint16_t power_bar_value() const { return power_raw_tenths() / 10; }
  uint16_t power_bar_max() const { return full_power() ? 1200 : 600; }
  const char *power_value_suffix() const { return " W pep"; }
  const char *power_scale_label(uint8_t index) const
  {
    static const char * const full_power_scale[5] = {"0", "300", "600", "900", "1200"};
    static const char * const half_power_scale[5] = {"0", "150", "300", "450", "600"};
    return (full_power() ? full_power_scale : half_power_scale)[index < 5 ? index : 0];
  }

  const char *pa_label() const { return shows_output_current() ? "  I PA" : "V PA"; }
  uint16_t pa_raw_tenths() const { return shows_output_current() ? packet_.current : packet_.voltage; }
  uint16_t pa_bar_value() const { return pa_raw_tenths() / 10; }
  uint16_t pa_bar_max() const { return (!shows_output_current() && full_power()) ? 60 : 50; }
  const char *pa_value_suffix() const { return shows_output_current() ? " A" : " V"; }
  const char *pa_scale_label(uint8_t index) const
  {
    static const char * const pa_50_scale[5] = {"0", "12.5", "25", "37.5", "50"};
    static const char * const v_60_scale[5] = {"0", "15", "30", "45", "60"};
    return ((!shows_output_current() && full_power()) ? v_60_scale : pa_50_scale)[index < 5 ? index : 0];
  }

private:
  const Expert_Packet &packet_;
};

#endif
