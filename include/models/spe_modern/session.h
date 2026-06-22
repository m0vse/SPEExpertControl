/*
 * Placeholder serial session for newer SPE Expert amplifier protocols.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include "models/amplifier_session.h"

class SpeModernSerialSession : public AmplifierSerialSession {
public:
  void begin() override;
  int available() override;
  AppAmpSerialPort serial_port() const override;
  bool amp_uses_usb_serial() const override;
  bool usb_console_active() const override;
  void release_usb_console() override;
  bool service_rx(AmplifierRuntime *runtime) override;
  bool process_next_ui_packet(AmplifierSessionPacketHandler handler) override;
};
