/*
 * Public WiFi setup API for popup visibility, saved credentials, and connection state.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

/**
 * Create the hidden WiFi setup popup and load any saved credentials.
 */
void wifi_setup_create(void);

/**
 * Run a blocking WiFi scan and update the popup network list.
 */
void wifi_setup_scan_networks(void);

/**
 * Advance WiFi connection state, including saved-credential background reconnect.
 */
void wifi_setup_service(void);

/**
 * Show or hide the WiFi setup popup.
 */
void wifi_setup_set_visible(bool visible);

/**
 * @return true when the WiFi stack currently reports WL_CONNECTED.
 */
bool wifi_setup_is_connected(void);

/**
 * @return true when a saved SSID is available in QSPI settings storage.
 */
bool wifi_setup_has_saved_credentials(void);

/**
 * Remove saved WiFi credentials from QSPI settings storage.
 */
void wifi_setup_clear_saved_credentials(void);

/**
 * Print saved credential state to the debug serial console without printing the password.
 */
void wifi_setup_print_saved_credentials(void);
