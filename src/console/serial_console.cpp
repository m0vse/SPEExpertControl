/*
 * Line-oriented debug serial console service.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "console/serial_console.h"

static unsigned long last_status_ms = 0;
static char line_buffer[96];
static uint8_t line_len = 0;
static bool poll_enabled = false;
static bool last_was_line_end = false;

void serial_console_service(ConsoleCommandHandler command_handler, ConsolePollHandler poll_handler)
{
  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\r' || c == '\n') {
      if (last_was_line_end) {
        continue;
      }
      last_was_line_end = true;
      line_buffer[line_len] = '\0';
      if (command_handler) {
        command_handler(line_buffer);
      }
      line_len = 0;
      continue;
    }

    last_was_line_end = false;
    if (c == '\b' || c == 0x7f) {
      if (line_len > 0) {
        --line_len;
      }
      continue;
    }

    if (line_len < sizeof(line_buffer) - 1) {
      line_buffer[line_len++] = c;
    }
  }

  if (!poll_enabled || !poll_handler) {
    return;
  }

  const unsigned long now = millis();
  if (now - last_status_ms < 2000) {
    return;
  }

  last_status_ms = now;
  poll_handler(now);
}

void serial_console_set_poll_enabled(bool enabled)
{
  poll_enabled = enabled;
}

bool serial_console_poll_enabled()
{
  return poll_enabled;
}
