/*
 * Shared lock for serialising access to the Arduino WiFi stack.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

class WifiStackLock {
public:
    WifiStackLock();
    ~WifiStackLock();

    WifiStackLock(const WifiStackLock &) = delete;
    WifiStackLock &operator=(const WifiStackLock &) = delete;
};
