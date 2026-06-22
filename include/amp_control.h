/*
 * Public amplifier control API used by the UI, serial console, and web control server.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

class AmplifierRuntime;

/**
 * Bind the active amplifier model runtime used by the public control facade.
 */
void amp_control_bind_runtime(AmplifierRuntime *runtime);

/**
 * Send one named front-panel key command to the amplifier.
 *
 * @param name Web/console key name such as "on", "off", "set", or "operate".
 * @return true when the name was recognised and a command was queued.
 */
bool amp_control_press_key(const char *name);

/**
 * Queue the remote-control ON command and assert DTR for amplifier wake.
 *
 * @return true when the command was queued.
 */
bool amp_control_power_on();

/**
 * Queue the periodic remote-control refresh command sequence.
 *
 * @return true when the refresh commands were queued.
 */
bool amp_control_refresh_remote();

/**
 * @return true while automatic RCU_ON keepalive polling should continue.
 */
bool amp_control_remote_updates_enabled();

/**
 * Send one queued model-specific command frame, if one is pending.
 */
void amp_control_process_next_queued_command();
