/*
 * Streaming parser for SPE Expert 1K-FA serial packets.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/spe_expert1k/packet_parser.h"

ExpertPacketParser::Result ExpertPacketParser::read(uint8_t value)
{
  switch (state_) {
    case State::Sync:
      if (value != 0xAA) {
        bytes_ = 0x00;
      } else if (bytes_ == 2) {
        state_ = State::Len;
      } else {
        bytes_++;
      }
      break;
    case State::Len:
      if (value == 0x6a) {
        frame_type_ = value;
        length_ = MODERN_RCU_PAYLOAD_LEN;
        bytes_ = 0x00;
        state_ = State::ModernRcuData;
        break;
      }
      if (value == 1 || value == MAX_DATA || value == EXPERT_PACKET_MAX_LEN) {
        length_ = value;
        state_ = State::Data;
      } else {
        resetFromUnexpected(value);
      }
      bytes_ = 0x00;
      break;
    case State::ModernRcuData:
      if (bytes_ < sizeof(data_)) {
        data_[bytes_] = value;
      }
      ++bytes_;
      if (bytes_ == length_) {
        resetState();
        return Result::ModernRcuFrame;
      }
      break;
    case State::Data:
      data_[bytes_] = value;
      if (bytes_ < sizeof(packet_)) {
        reinterpret_cast<uint8_t *>(&packet_)[bytes_] = value;
      }
      ++bytes_;
      checksum_ += value;
      if (bytes_ == length_) {
        state_ = State::Sum;
      }
      break;
    case State::Sum:
      if (length_ == EXPERT_PACKET_MAX_LEN) {
        checksum_lo_ = value;
        state_ = State::SumHi;
        break;
      }
      if (static_cast<uint8_t>(checksum_) == value) {
        resetState();
        return Result::PacketReady;
      }
      invalid_length_ = length_;
      invalid_expected_checksum_ = static_cast<uint8_t>(checksum_);
      invalid_received_checksum_ = value;
      resetFromUnexpected(value);
      return Result::InvalidChecksum;
    case State::SumHi:
    {
      const uint16_t received = static_cast<uint16_t>(checksum_lo_) | (static_cast<uint16_t>(value) << 8);
      if (checksum_ == received) {
        state_ = State::Cr;
        break;
      }
      invalid_length_ = length_;
      invalid_expected_checksum_ = static_cast<uint8_t>(checksum_ & 0xff);
      invalid_received_checksum_ = checksum_lo_;
      resetFromUnexpected(value);
      return Result::InvalidChecksum;
    }
    case State::Cr:
      if (value == 13) {
        state_ = State::Lf;
      } else {
        resetFromUnexpected(value);
      }
      break;
    case State::Lf:
      if (value == 10) {
        resetState();
        return Result::PacketReady;
      }
      resetFromUnexpected(value);
      break;
  }

  return Result::None;
}

void ExpertPacketParser::reset()
{
  resetState();
  length_ = 0x00;
}

void ExpertPacketParser::resetState()
{
  bytes_ = 0x00;
  checksum_ = 0x00;
  checksum_lo_ = 0x00;
  state_ = State::Sync;
}

void ExpertPacketParser::resetFromUnexpected(uint8_t value)
{
  reset();
  if (value == 0xAA) {
    bytes_ = 0x01;
  }
}
