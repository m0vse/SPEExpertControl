/*
 * Generic serial transport health counters used by amplifier protocol handlers.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <Arduino.h>

struct SerialTransportStats {
  uint32_t rx_bytes = 0;
  uint32_t valid_packets = 0;
  uint32_t invalid_checksums = 0;
  uint32_t commands_sent = 0;
  uint32_t max_available = 0;
  uint32_t max_task_gap_ms = 0;
  uint32_t max_rx_byte_gap_us = 0;
  uint32_t max_drain_burst = 0;
  uint32_t queued_packets_dropped = 0;
  uint32_t max_queue_depth = 0;
  uint32_t queued_commands_dropped = 0;
  uint32_t max_command_queue_depth = 0;
  uint32_t commands_suppressed = 0;
  uint32_t last_command_ms = 0;
  uint32_t modern_refreshes = 0;
  uint32_t last_modern_refresh_ms = 0;
  uint32_t last_checksum_error_ms = 0;
  uint32_t checksum_sync_resyncs = 0;
  uint32_t last_bad_available = 0;
  uint8_t last_command_opcode = 0;
  uint8_t last_bad_packet_len = 0;
  uint8_t last_bad_checksum_expected = 0;
  uint8_t last_bad_checksum_received = 0;
};

SerialTransportStats serial_transport_stats_snapshot();
void serial_transport_note_available(uint32_t available);
void serial_transport_note_task_gap(uint32_t gap_ms);
void serial_transport_note_drain_burst(uint32_t bytes);
void serial_transport_note_rx_byte(uint32_t now_us);
void serial_transport_note_valid_packet();
void serial_transport_note_invalid_checksum(uint8_t len, uint8_t expected, uint8_t received, uint32_t available);
void serial_transport_note_command(uint8_t opcode);
void serial_transport_note_command_suppressed(uint8_t opcode);
void serial_transport_note_modern_refresh();
void serial_transport_note_packet_drop();
void serial_transport_note_queue_depth(uint32_t depth);
void serial_transport_note_command_drop();
void serial_transport_note_command_queue_depth(uint32_t depth);
