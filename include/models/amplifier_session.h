/*
 * Generic amplifier serial session contract for model-specific protocols.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <stdint.h>

#include "app_config.h"

class AmplifierRuntime;

struct AmplifierSessionPacket {
  const void *data = nullptr;
  uint8_t len = 0;
};

using AmplifierSessionPacketHandler = void (*)(const AmplifierSessionPacket &packet);

class AmplifierSerialSession {
public:
  virtual ~AmplifierSerialSession() = default;

  virtual void begin() = 0;
  virtual int available() = 0;
  virtual AppAmpSerialPort serial_port() const = 0;
  virtual bool amp_uses_usb_serial() const = 0;
  virtual bool usb_console_active() const = 0;
  virtual void release_usb_console() = 0;
  virtual bool service_rx(AmplifierRuntime *runtime) = 0;
  virtual bool process_next_ui_packet(AmplifierSessionPacketHandler handler) = 0;
};
