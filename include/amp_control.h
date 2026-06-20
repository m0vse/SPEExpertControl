/*
 * Public amplifier control API used by the UI, serial console, and web control server.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

/**
 * Send one named front-panel key command to the amplifier.
 *
 * @param name Web/console key name such as "on", "off", "set", or "operate".
 * @return true when the name was recognised and a command was queued.
 */
bool amp_control_press_key(const char *name);
