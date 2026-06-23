/*
 * Public WiFi setup API for popup visibility, saved credentials, and connection state.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <stdint.h>

struct WifiHealthSnapshot {
    uint32_t health_checks;
    uint32_t gateway_ping_failures;
    uint32_t watchdog_reconnects;
    unsigned long last_health_check_ms;
    unsigned long last_health_failure_ms;
    int last_status;
    int last_gateway_ping_ms;
    uint8_t consecutive_gateway_failures;
    uint8_t gateway_failure_threshold;
    unsigned long check_interval_ms;
};

/**
 * Create the hidden WiFi setup popup and load any saved credentials.
 */
void wifi_setup_create(void);

/**
 * Queue a WiFi scan and update the popup network list when the result is ready.
 */
void wifi_setup_scan_networks(void);

/**
 * Apply pending WiFi UI updates and animate visible connection progress.
 */
void wifi_setup_service(void);

/**
 * Advance WiFi connection state, including saved-credential background reconnect.
 *
 * This can block inside the WiFi stack and must not be called while holding the
 * LVGL mutex.
 */
void wifi_setup_connection_service(void);

/**
 * Periodically check connected WiFi reachability and force reconnect if needed.
 */
void wifi_setup_health_service(void);

/**
 * Show or hide the WiFi setup popup.
 */
void wifi_setup_set_visible(bool visible);

/**
 * Refresh the LCD sleep timeout field from saved controller configuration.
 *
 * Call this while holding the LVGL lock when invoked outside the UI task.
 */
void wifi_setup_refresh_screensaver_timeout(void);

/**
 * @return true when the hidden setup popup is currently visible.
 */
bool wifi_setup_is_visible(void);

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
 * Force the WiFi state machine to disconnect and reconnect using saved credentials.
 */
void wifi_setup_force_reconnect(void);

/**
 * Return WiFi gateway watchdog counters for serial diagnostics.
 */
WifiHealthSnapshot wifi_setup_health_snapshot(void);

/**
 * Print saved credential state to the debug serial console without printing the password.
 */
void wifi_setup_print_saved_credentials(void);
