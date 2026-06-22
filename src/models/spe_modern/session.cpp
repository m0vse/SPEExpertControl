/*
 * Serial session for newer SPE Expert amplifier protocols.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/spe_modern/session.h"

#include "app_config.h"
#include "models/amplifier_model.h"
#include "models/amplifier_runtime.h"
#include "models/amplifier_runtime_manager.h"
#include "models/spe_expert1k/packet_parser.h"
#include "models/spe_expert1k/serial_link.h"
#include "models/spe_modern/runtime.h"
#include "serial/transport_stats.h"

#include <Arduino.h>

void SpeModernSerialSession::begin()
{
  spe_expert1k_serial_begin();
}

int SpeModernSerialSession::available()
{
  return spe_expert1k_serial_available();
}

AppAmpSerialPort SpeModernSerialSession::serial_port() const
{
  return app_config_amp_serial_port();
}

bool SpeModernSerialSession::amp_uses_usb_serial() const
{
  return spe_expert1k_amp_uses_usb_serial();
}

bool SpeModernSerialSession::usb_console_active() const
{
  return spe_expert1k_usb_console_active();
}

void SpeModernSerialSession::release_usb_console()
{
  spe_expert1k_usb_console_release();
}

bool SpeModernSerialSession::service_rx(AmplifierRuntime *runtime)
{
  uint32_t drained_bytes = 0;
  bool completed_packet = false;
  SpeModernRuntime *modern_runtime = runtime && runtime->model_id() != AmpModelId::SpeExpert1k
    ? static_cast<SpeModernRuntime *>(runtime)
    : nullptr;

  while (true) {
    SpeExpert1kReadResult read_result;
    if (!spe_expert1k_serial_read(read_result)) {
      break;
    }
    ++drained_bytes;

    switch (read_result.result) {
      case ExpertPacketParser::Result::PacketReady:
        serial_transport_note_valid_packet();
        completed_packet = true;
        amplifier_runtime_note_detected_model(amp_model_detect_from_packet(read_result.raw, read_result.len));
        if (read_result.len == 67 && modern_runtime) {
          modern_runtime->process_status_packet(read_result.raw, read_result.len, millis());
        } else if (runtime) {
          runtime->mark_activity(millis());
        }
        break;
      case ExpertPacketParser::Result::ModernRcuFrame:
        serial_transport_note_valid_packet();
        completed_packet = true;
        amplifier_runtime_note_detected_model(amp_model_detect_from_packet(read_result.raw, read_result.len));
        if (modern_runtime) {
          modern_runtime->process_rcu_frame(read_result.raw, read_result.len, millis());
        } else if (runtime) {
          runtime->mark_activity(millis());
        }
        break;
      case ExpertPacketParser::Result::InvalidChecksum:
        serial_transport_note_invalid_checksum(
          read_result.invalid_len,
          read_result.invalid_expected_checksum,
          read_result.invalid_received_checksum,
          read_result.last_available);
        break;
      case ExpertPacketParser::Result::None:
        break;
    }
  }

  serial_transport_note_drain_burst(drained_bytes);
  return completed_packet;
}

bool SpeModernSerialSession::process_next_ui_packet(AmplifierSessionPacketHandler handler)
{
  (void)handler;
  return false;
}
