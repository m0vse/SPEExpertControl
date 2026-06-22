/*
 * SPE Expert 1K-FA serial session adapter for the generic amplifier session API.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include "models/amplifier_session.h"

class SpeExpert1kSerialSession : public AmplifierSerialSession {
public:
  void begin() override;
  int available() override;
  AppAmpSerialPort serial_port() const override;
  bool amp_uses_usb_serial() const override;
  bool usb_console_active() const override;
  void release_usb_console() override;
  bool service_rx(AmplifierRuntime *runtime) override;
  bool process_next_ui_packet(AmplifierSessionPacketHandler handler) override;

private:
  void process_response_packet(const AmplifierSessionPacket &packet) const;
};
