/*
 * Shared lock for serialising access to the Arduino WiFi stack.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "network/wifi_lock.h"

#include <mbed.h>

static rtos::Mutex wifi_stack_mutex;

WifiStackLock::WifiStackLock()
{
    wifi_stack_mutex.lock();
}

WifiStackLock::~WifiStackLock()
{
    wifi_stack_mutex.unlock();
}
