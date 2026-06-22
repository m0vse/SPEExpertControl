/*
 * SPE Expert 1K-FA serial session adapter for the generic amplifier session API.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/spe_expert1k/session.h"

#include <Arduino.h>

#include "models/amplifier_runtime.h"
#include "models/amplifier_runtime_manager.h"
#include "models/spe_expert1k/expertpackets.h"
#include "models/spe_expert1k/packet_parser.h"
#include "models/spe_expert1k/serial_link.h"
#include "serial/transport_stats.h"

void SpeExpert1kSerialSession::begin()
{
  spe_expert1k_serial_begin();
}

int SpeExpert1kSerialSession::available()
{
  return spe_expert1k_serial_available();
}

AppAmpSerialPort SpeExpert1kSerialSession::serial_port() const
{
  return spe_expert1k_amp_serial_port();
}

bool SpeExpert1kSerialSession::amp_uses_usb_serial() const
{
  return spe_expert1k_amp_uses_usb_serial();
}

bool SpeExpert1kSerialSession::usb_console_active() const
{
  return spe_expert1k_usb_console_active();
}

void SpeExpert1kSerialSession::release_usb_console()
{
  spe_expert1k_usb_console_release();
}

bool SpeExpert1kSerialSession::service_rx(AmplifierRuntime *runtime)
{
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
        serial_transport_note_valid_packet();
        completed_packet = true;
        amplifier_runtime_note_detected_model(amp_model_detect_from_packet(read_result.raw, read_result.len));
        if (read_result.len == 30) {
          if (runtime) {
            runtime->mark_activity(millis());
          }
          spe_expert1k_queue_packet(read_result.packet, read_result.len);
        } else if (read_result.len == EXPERT_PACKET_MAX_LEN) {
          if (runtime) {
            runtime->mark_activity(millis());
          }
        } else {
          const AmplifierSessionPacket packet{&read_result.packet, static_cast<uint8_t>(read_result.len)};
          process_response_packet(packet);
        }
        break;
      case ExpertPacketParser::Result::ModernRcuFrame:
        serial_transport_note_valid_packet();
        completed_packet = true;
        amplifier_runtime_note_detected_model(amp_model_detect_from_packet(read_result.raw, read_result.len));
        if (runtime) {
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

bool SpeExpert1kSerialSession::process_next_ui_packet(AmplifierSessionPacketHandler handler)
{
  if (!handler) {
    return false;
  }

  SpeExpert1kQueuedPacket queued;
  if (!spe_expert1k_dequeue_packet(queued)) {
    return false;
  }

  const AmplifierSessionPacket packet{&queued.packet, queued.len};
  handler(packet);
  return true;
}

void SpeExpert1kSerialSession::process_response_packet(const AmplifierSessionPacket &packet) const
{
  if (!packet.data || packet.len != 1) {
    return;
  }
  if (spe_expert1k_amp_uses_usb_serial() && !spe_expert1k_usb_console_active()) {
    return;
  }

  const Expert_Packet &expert_packet = *static_cast<const Expert_Packet *>(packet.data);
  if (expert_packet.status_code == 0x15) {
    Serial.println("NAK");
  } else if (expert_packet.status_code == 0xff) {
    Serial.println("UNK");
  } else if (expert_packet.status_code != 0x06) {
    Serial.print(F("Unexpected 1-byte response: 0x"));
    Serial.println(expert_packet.status_code, HEX);
  }
}
