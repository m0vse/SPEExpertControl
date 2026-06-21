/*
 * Generic serial transport health counters used by amplifier protocol handlers.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "serial/transport_stats.h"

#include <rtos.h>

static rtos::Mutex stats_mutex;
static SerialTransportStats stats;
static uint32_t last_rx_us = 0;

class StatsLock {
public:
  StatsLock() { stats_mutex.lock(); }
  ~StatsLock() { stats_mutex.unlock(); }
};

SerialTransportStats serial_transport_stats_snapshot()
{
  StatsLock lock;
  return stats;
}

void serial_transport_note_available(uint32_t available)
{
  StatsLock lock;
  if (available > stats.max_available) {
    stats.max_available = available;
  }
}

void serial_transport_note_task_gap(uint32_t gap_ms)
{
  StatsLock lock;
  if (gap_ms > stats.max_task_gap_ms) {
    stats.max_task_gap_ms = gap_ms;
  }
}

void serial_transport_note_drain_burst(uint32_t bytes)
{
  StatsLock lock;
  if (bytes > stats.max_drain_burst) {
    stats.max_drain_burst = bytes;
  }
}

void serial_transport_note_rx_byte(uint32_t now_us)
{
  StatsLock lock;
  ++stats.rx_bytes;
  if (last_rx_us != 0) {
    const uint32_t gap_us = now_us - last_rx_us;
    if (gap_us > stats.max_rx_byte_gap_us) {
      stats.max_rx_byte_gap_us = gap_us;
    }
  }
  last_rx_us = now_us;
}

void serial_transport_note_valid_packet()
{
  StatsLock lock;
  ++stats.valid_packets;
}

void serial_transport_note_invalid_checksum(uint8_t len, uint8_t expected, uint8_t received, uint32_t available)
{
  StatsLock lock;
  ++stats.invalid_checksums;
  stats.last_checksum_error_ms = millis();
  if (received == 0xAA) {
    ++stats.checksum_sync_resyncs;
  }
  stats.last_bad_available = available;
  stats.last_bad_packet_len = len;
  stats.last_bad_checksum_expected = expected;
  stats.last_bad_checksum_received = received;
}

void serial_transport_note_command()
{
  StatsLock lock;
  ++stats.commands_sent;
}

void serial_transport_note_packet_drop()
{
  StatsLock lock;
  ++stats.queued_packets_dropped;
}

void serial_transport_note_queue_depth(uint32_t depth)
{
  StatsLock lock;
  if (depth > stats.max_queue_depth) {
    stats.max_queue_depth = depth;
  }
}

void serial_transport_note_command_drop()
{
  StatsLock lock;
  ++stats.queued_commands_dropped;
}

void serial_transport_note_command_queue_depth(uint32_t depth)
{
  StatsLock lock;
  if (depth > stats.max_command_queue_depth) {
    stats.max_command_queue_depth = depth;
  }
}
