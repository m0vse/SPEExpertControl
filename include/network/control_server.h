/*
 * Public API for servicing the lightweight HTTP control server.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <Arduino.h>

/**
 * Service one pass of the lightweight HTTP control server.
 *
 * This function is intentionally non-blocking when WiFi is disconnected and is
 * expected to be called repeatedly from the background serial/network task.
 */
void control_server_service(void);

/**
 * Print lightweight HTTP server counters to a debug console or other Print sink.
 */
void control_server_print_stats(Print &out);
