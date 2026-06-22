/*
 * SPE Expert 1K-FA serial link queues, byte parser integration, and TX framing.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/spe_expert1k/serial_link.h"

#include "app_config.h"
#include "serial/transport_stats.h"
#include <rtos.h>

#ifndef SPE_VERBOSE_PACKET_LOG
#define SPE_VERBOSE_PACKET_LOG 0
#endif

static const uint8_t AMP_PACKET_QUEUE_SIZE = 8;
static const uint8_t AMP_COMMAND_QUEUE_SIZE = 16;
static const uint8_t AMP_COMMAND_MAX_LEN = 4;

struct QueuedAmpCommand {
  uint8_t data[AMP_COMMAND_MAX_LEN]{};
  uint8_t len = 0;
};

static SpeExpert1kQueuedPacket packet_queue[AMP_PACKET_QUEUE_SIZE];
static uint8_t packet_queue_head = 0;
static uint8_t packet_queue_tail = 0;
static uint8_t packet_queue_count = 0;
static QueuedAmpCommand command_queue[AMP_COMMAND_QUEUE_SIZE];
static uint8_t command_queue_head = 0;
static uint8_t command_queue_tail = 0;
static uint8_t command_queue_count = 0;
static rtos::Mutex serial_mutex;
static rtos::Mutex packet_queue_mutex;
static rtos::Mutex command_queue_mutex;
static ExpertPacketParser parser;
static uint32_t last_available = 0;
static uint8_t usb_escape_count = 0;
static bool usb_console_active = false;
static AppAmpSerialPort amp_serial_port_selection = AppAmpSerialPort::Uart1;
static uint32_t amp_baud = APP_CONFIG_DEFAULT_AMP_BAUD;

class SerialLock {
public:
  SerialLock() { serial_mutex.lock(); }
  ~SerialLock() { serial_mutex.unlock(); }
};

class PacketQueueLock {
public:
  PacketQueueLock() { packet_queue_mutex.lock(); }
  ~PacketQueueLock() { packet_queue_mutex.unlock(); }
};

class CommandQueueLock {
public:
  CommandQueueLock() { command_queue_mutex.lock(); }
  ~CommandQueueLock() { command_queue_mutex.unlock(); }
};

static Stream &amp_serial_port()
{
  switch (amp_serial_port_selection) {
    case AppAmpSerialPort::Uart1: return Serial1;
    case AppAmpSerialPort::Uart2: return Serial2;
    case AppAmpSerialPort::Uart3: return Serial3;
    case AppAmpSerialPort::Uart4: return Serial4;
    case AppAmpSerialPort::Usb: return Serial;
    default: return Serial1;
  }
}

static bool amp_serial_is_usb()
{
  return amp_serial_port_selection == AppAmpSerialPort::Usb;
}

static void begin_uart_port(uint32_t baud)
{
  switch (amp_serial_port_selection) {
    case AppAmpSerialPort::Uart1:
      Serial1.begin(baud);
      break;
    case AppAmpSerialPort::Uart2:
      Serial2.begin(baud);
      break;
    case AppAmpSerialPort::Uart3:
      Serial3.begin(baud);
      break;
    case AppAmpSerialPort::Uart4:
      Serial4.begin(baud);
      break;
    case AppAmpSerialPort::Usb:
    default:
      break;
  }
}

static void set_usb_console_active_locked(bool active)
{
  usb_console_active = active;
}

static bool dequeue_command(QueuedAmpCommand &queued)
{
  CommandQueueLock lock;
  if (command_queue_count == 0) {
    return false;
  }

  queued = command_queue[command_queue_tail];
  command_queue_tail = (command_queue_tail + 1) % AMP_COMMAND_QUEUE_SIZE;
  --command_queue_count;
  return true;
}

static void send_command_frame(const uint8_t *cmd, uint8_t len)
{
  SerialLock lock;
  if (amp_serial_is_usb() && usb_console_active) {
    return;
  }
  serial_transport_note_command();
  Stream &port = amp_serial_port();
  port.write(0x55);
  port.write(0x55);
  port.write(0x55);
  port.write(len & 0xff);
  uint8_t sum = 0;
  for (uint8_t i = 0; i < len; ++i) {
    const uint8_t c = cmd[i];
    port.write(c);
    sum += c;
  }
  port.write(sum);

#if SPE_VERBOSE_PACKET_LOG
  Serial.print(F("TX command:"));
  for (uint8_t i = 0; i < len; ++i) {
    Serial.print(' ');
    Serial.print(cmd[i], HEX);
  }
  Serial.println();
#endif
}

void spe_expert1k_serial_begin()
{
  SerialLock lock;
  amp_serial_port_selection = app_config_amp_serial_port();
  amp_baud = app_config_amp_baud();
  usb_escape_count = 0;
  usb_console_active = false;
  if (!amp_serial_is_usb()) {
    begin_uart_port(amp_baud);
  }
}

int spe_expert1k_serial_available()
{
  SerialLock lock;
  if (amp_serial_is_usb() && usb_console_active) {
    return 0;
  }
  Stream &port = amp_serial_port();
  const int available_bytes = port.available();
  last_available = available_bytes < 0 ? 0 : static_cast<uint32_t>(available_bytes);
  serial_transport_note_available(last_available);
  return available_bytes;
}

bool spe_expert1k_serial_read(SpeExpert1kReadResult &result)
{
  result = {};
  int value = -1;
  {
    SerialLock lock;
    if (amp_serial_is_usb() && usb_console_active) {
      return false;
    }
    Stream &port = amp_serial_port();
    const int available_bytes = port.available();
    last_available = available_bytes < 0 ? 0 : static_cast<uint32_t>(available_bytes);
    serial_transport_note_available(last_available);
    if (available_bytes <= 0) {
      return false;
    }
    value = port.read();
  }

  if (amp_serial_is_usb()) {
    if (value == 0x1b) {
      if (++usb_escape_count >= 3) {
        {
          SerialLock lock;
          set_usb_console_active_locked(true);
        }
        usb_escape_count = 0;
        Serial.println();
        Serial.println(F("USB serial console active. Type 'exit' to return USB to amplifier comms."));
      }
      return true;
    }
    usb_escape_count = 0;
  }

  serial_transport_note_rx_byte(micros());
  result.result = parser.read(static_cast<uint8_t>(value));
  result.packet = parser.packet();
  result.len = parser.length();
  result.invalid_len = parser.invalidLength();
  result.invalid_expected_checksum = parser.invalidExpectedChecksum();
  result.invalid_received_checksum = parser.invalidReceivedChecksum();
  result.last_available = last_available;
  return true;
}

AppAmpSerialPort spe_expert1k_amp_serial_port()
{
  return amp_serial_port_selection;
}

bool spe_expert1k_amp_uses_usb_serial()
{
  return amp_serial_is_usb();
}

bool spe_expert1k_usb_console_active()
{
  SerialLock lock;
  return amp_serial_is_usb() && usb_console_active;
}

void spe_expert1k_usb_console_release()
{
  SerialLock lock;
  if (!amp_serial_is_usb()) {
    return;
  }
  usb_escape_count = 0;
  set_usb_console_active_locked(false);
}

bool spe_expert1k_queue_command(std::initializer_list<uint8_t> cmd)
{
  if (cmd.size() == 0 || cmd.size() > AMP_COMMAND_MAX_LEN) {
    return false;
  }

  uint8_t depth = 0;
  {
    CommandQueueLock lock;
    if (command_queue_count >= AMP_COMMAND_QUEUE_SIZE) {
      serial_transport_note_command_drop();
      return false;
    }

    QueuedAmpCommand &queued = command_queue[command_queue_head];
    queued.len = static_cast<uint8_t>(cmd.size());
    uint8_t i = 0;
    for (uint8_t c : cmd) {
      queued.data[i++] = c;
    }

    command_queue_head = (command_queue_head + 1) % AMP_COMMAND_QUEUE_SIZE;
    ++command_queue_count;
    depth = command_queue_count;
  }

  serial_transport_note_command_queue_depth(depth);
  return true;
}

void spe_expert1k_process_next_queued_command()
{
  QueuedAmpCommand queued;
  if (dequeue_command(queued)) {
    send_command_frame(queued.data, queued.len);
  }
}

void spe_expert1k_queue_packet(const Expert_Packet &packet, uint8_t len)
{
  uint8_t depth = 0;
  {
    PacketQueueLock lock;
    if (packet_queue_count >= AMP_PACKET_QUEUE_SIZE) {
      packet_queue_tail = (packet_queue_tail + 1) % AMP_PACKET_QUEUE_SIZE;
      --packet_queue_count;
      serial_transport_note_packet_drop();
    }

    packet_queue[packet_queue_head].packet = packet;
    packet_queue[packet_queue_head].len = len;
    packet_queue_head = (packet_queue_head + 1) % AMP_PACKET_QUEUE_SIZE;
    ++packet_queue_count;
    depth = packet_queue_count;
  }

  serial_transport_note_queue_depth(depth);
}

bool spe_expert1k_dequeue_packet(SpeExpert1kQueuedPacket &queued)
{
  PacketQueueLock lock;
  if (packet_queue_count == 0) {
    return false;
  }

  queued = packet_queue[packet_queue_tail];
  packet_queue_tail = (packet_queue_tail + 1) % AMP_PACKET_QUEUE_SIZE;
  --packet_queue_count;
  return true;
}
