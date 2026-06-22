/*
 * Generic amplifier bootstrap serial detector.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/bootstrap/session.h"

#include "models/amplifier_model.h"
#include "models/amplifier_runtime.h"
#include "models/amplifier_runtime_manager.h"
#include "models/spe_expert1k/packet_parser.h"
#include "models/spe_expert1k/serial_link.h"
#include "models/spe_expert1k/runtime.h"
#include "models/spe_modern/runtime.h"
#include "serial/transport_stats.h"

#include <Arduino.h>

void BootstrapSerialSession::begin()
{
  spe_expert1k_serial_begin();
}

int BootstrapSerialSession::available()
{
  return spe_expert1k_serial_available();
}

AppAmpSerialPort BootstrapSerialSession::serial_port() const
{
  return spe_expert1k_amp_serial_port();
}

bool BootstrapSerialSession::amp_uses_usb_serial() const
{
  return spe_expert1k_amp_uses_usb_serial();
}

bool BootstrapSerialSession::usb_console_active() const
{
  return spe_expert1k_usb_console_active();
}

void BootstrapSerialSession::release_usb_console()
{
  spe_expert1k_usb_console_release();
}

static void forward_detected_packet(const SpeExpert1kReadResult &read_result, unsigned long now)
{
  const AmpModelId detected = amp_model_detect_from_packet(read_result.raw, read_result.len);
  if (detected == AmpModelId::Unknown) {
    return;
  }

  amplifier_runtime_note_detected_model(detected);
  AmplifierRuntime *runtime = amplifier_runtime_active();
  if (!runtime) {
    return;
  }

  if (detected == AmpModelId::SpeExpert1k && read_result.result == ExpertPacketParser::Result::PacketReady &&
      read_result.len == 30) {
    runtime->mark_activity(now);
    spe_expert1k_queue_packet(read_result.packet, read_result.len);
    return;
  }

  if (runtime->model_id() != AmpModelId::SpeExpert1k) {
    SpeModernRuntime *modern_runtime = static_cast<SpeModernRuntime *>(runtime);
    if (read_result.result == ExpertPacketParser::Result::ModernRcuFrame) {
      modern_runtime->process_rcu_frame(read_result.raw, read_result.len, now);
    } else if (read_result.result == ExpertPacketParser::Result::PacketReady && read_result.len == 67) {
      modern_runtime->process_status_packet(read_result.raw, read_result.len, now);
    }
  }
}

bool BootstrapSerialSession::service_rx(AmplifierRuntime *runtime)
{
  (void)runtime;
  uint32_t drained_bytes = 0;
  bool completed_packet = false;

  while (true) {
    SpeExpert1kReadResult read_result;
    if (!spe_expert1k_serial_read(read_result)) {
      break;
    }
    ++drained_bytes;

    switch (read_result.result) {
      case ExpertPacketParser::Result::PacketReady:
      case ExpertPacketParser::Result::ModernRcuFrame:
        serial_transport_note_valid_packet();
        completed_packet = true;
        forward_detected_packet(read_result, millis());
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

bool BootstrapSerialSession::process_next_ui_packet(AmplifierSessionPacketHandler handler)
{
  (void)handler;
  return false;
}
