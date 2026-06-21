/*
 * Line-oriented debug serial console service.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <Arduino.h>

using ConsoleCommandHandler = void (*)(char *line);
using ConsolePollHandler = void (*)(unsigned long now);

void serial_console_service(ConsoleCommandHandler command_handler, ConsolePollHandler poll_handler);
void serial_console_set_poll_enabled(bool enabled);
bool serial_console_poll_enabled();
