/*
 * SPE Expert 1K-FA serial link queues, byte parser integration, and TX framing.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <stdint.h>

#include "app_config.h"
#include "models/spe_expert1k/expertpackets.h"
#include "models/spe_expert1k/packet_parser.h"
#include <Arduino.h>
#include <initializer_list>

struct SpeExpert1kQueuedPacket {
  Expert_Packet packet{};
  uint8_t len = 0;
};

struct SpeExpert1kReadResult {
  ExpertPacketParser::Result result = ExpertPacketParser::Result::None;
  Expert_Packet packet{};
  uint8_t raw[EXPERT_PACKET_BUFFER_LEN] = {};
  uint16_t len = 0;
  uint8_t frame_type = 0;
  uint8_t invalid_len = 0;
  uint8_t invalid_expected_checksum = 0;
  uint8_t invalid_received_checksum = 0;
  uint32_t last_available = 0;
};

void spe_expert1k_serial_begin();
int spe_expert1k_serial_available();
bool spe_expert1k_serial_read(SpeExpert1kReadResult &result);
AppAmpSerialPort spe_expert1k_amp_serial_port();
bool spe_expert1k_amp_uses_usb_serial();
bool spe_expert1k_usb_console_active();
void spe_expert1k_usb_console_release();
bool spe_expert1k_queue_command(std::initializer_list<uint8_t> cmd);
void spe_expert1k_process_next_queued_command();
void spe_expert1k_queue_packet(const Expert_Packet &packet, uint8_t len);
bool spe_expert1k_dequeue_packet(SpeExpert1kQueuedPacket &queued);
