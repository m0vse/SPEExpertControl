/*
 * Placeholder serial session for newer SPE Expert amplifier protocols.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/spe_modern/session.h"

#include "app_config.h"

void SpeModernSerialSession::begin()
{
}

int SpeModernSerialSession::available()
{
  return 0;
}

AppAmpSerialPort SpeModernSerialSession::serial_port() const
{
  return app_config_amp_serial_port();
}

bool SpeModernSerialSession::amp_uses_usb_serial() const
{
  return app_config_amp_serial_port() == AppAmpSerialPort::Usb;
}

bool SpeModernSerialSession::usb_console_active() const
{
  return false;
}

void SpeModernSerialSession::release_usb_console()
{
}

bool SpeModernSerialSession::service_rx(AmplifierRuntime *runtime)
{
  (void)runtime;
  return false;
}

bool SpeModernSerialSession::process_next_ui_packet(AmplifierSessionPacketHandler handler)
{
  (void)handler;
  return false;
}
