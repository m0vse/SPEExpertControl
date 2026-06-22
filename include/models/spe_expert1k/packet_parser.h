/*
 * Streaming parser for SPE Expert 1K-FA serial packets.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <stdint.h>
#include "models/spe_expert1k/expertpackets.h"

static const uint8_t EXPERT_PACKET_MAX_LEN = 67;

class ExpertPacketParser {
public:
  enum class Result {
    None,
    PacketReady,
    ModernRcuFrame,
    InvalidChecksum
  };

  Result read(uint8_t value);

  const Expert_Packet &packet() const { return packet_; }
  const uint8_t *data() const { return data_; }
  uint8_t length() const { return length_; }
  uint8_t invalidLength() const { return invalid_length_; }
  uint8_t invalidExpectedChecksum() const { return invalid_expected_checksum_; }
  uint8_t invalidReceivedChecksum() const { return invalid_received_checksum_; }
  uint8_t frameType() const { return frame_type_; }

private:
  enum class State : uint8_t {
    Sync,
    Len,
    Data,
    Sum,
    SumHi,
    Cr,
    Lf
  };

  void reset();
  void resetState();
  void resetFromUnexpected(uint8_t value);

  State state_ = State::Sync;
  Expert_Packet packet_{};
  uint8_t data_[EXPERT_PACKET_MAX_LEN] = {};
  uint8_t bytes_ = 0x00;
  uint16_t checksum_ = 0x00;
  uint8_t checksum_lo_ = 0x00;
  uint8_t frame_type_ = 0x00;
  uint8_t length_ = 0x00;
  uint8_t invalid_length_ = 0x00;
  uint8_t invalid_expected_checksum_ = 0x00;
  uint8_t invalid_received_checksum_ = 0x00;
};
