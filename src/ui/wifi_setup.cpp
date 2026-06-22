/*
 * Hidden WiFi setup popup, saved credential storage, scanning, and background reconnect handling.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "ui/wifi_setup.h"

#include "app_config.h"
#include "display/lvgl_giga_display.h"
#include "network/wifi_lock.h"
#include <Arduino.h>
#include <WiFi.h>
#include <rtos.h>
#include <stdio.h>
#include <string.h>
#include <ui.h>

static const int MAX_WIFI_NETWORKS = 12;
static const int MAX_WIFI_SSID_LEN = 32;
static const int MAX_WIFI_PASSWORD_LEN = 64;
static const unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;
static const unsigned long WIFI_AUTOCONNECT_DELAY_MS = 2500;
static const unsigned long WIFI_RECONNECT_RETRY_MS = 30000;

static char wifi_ssids[MAX_WIFI_NETWORKS][MAX_WIFI_SSID_LEN + 1];
static char wifi_dropdown_options[(MAX_WIFI_SSID_LEN + 16) * MAX_WIFI_NETWORKS];
static char saved_ssid[MAX_WIFI_SSID_LEN + 1];
static char saved_password[MAX_WIFI_PASSWORD_LEN + 1];
static char pending_ssid[MAX_WIFI_SSID_LEN + 1];
static char pending_password[MAX_WIFI_PASSWORD_LEN + 1];
static int wifi_network_count = 0;
static bool wifi_setup_active = true;
static bool wifi_connecting = false;
static bool wifi_skipped = false;
static bool wifi_connected = false;
static bool wifi_stack_available = true;
static bool saved_credentials_loaded = false;
static bool saved_credentials_valid = false;
static bool pending_save_on_success = false;
static bool wifi_ui_dirty = false;
static bool wifi_begin_pending = false;
static bool wifi_hide_requested = false;
static bool wifi_scan_requested = false;
static bool wifi_scan_running = false;
static bool wifi_scan_result_pending = false;
static int wifi_scan_result = 0;
static int wifi_scan_count = 0;
static unsigned long wifi_connect_started = 0;
static unsigned long wifi_next_reconnect = WIFI_AUTOCONNECT_DELAY_MS;
static char wifi_status_text[96];
static char wifi_startup_text[96];
static char wifi_scan_ssids[MAX_WIFI_NETWORKS][MAX_WIFI_SSID_LEN + 1];
static long wifi_scan_rssi[MAX_WIFI_NETWORKS];
static rtos::Mutex wifi_state_mutex;
static lv_obj_t *wifi_panel = NULL;
static lv_obj_t *wifi_dropdown = NULL;
static lv_obj_t *wifi_password = NULL;
static lv_obj_t *display_flip_button = NULL;
static lv_obj_t *display_flip_label = NULL;
static lv_obj_t *amp_serial_dropdown = NULL;
static lv_obj_t *wifi_keyboard = NULL;
static lv_obj_t *wifi_status_label = NULL;
static lv_obj_t *wifi_hotspot = NULL;

static void wifi_connect_event_cb(lv_event_t *e);
static void wifi_rescan_event_cb(lv_event_t *e);
static void wifi_skip_event_cb(lv_event_t *e);
static void wifi_password_event_cb(lv_event_t *e);
static void display_flip_event_cb(lv_event_t *e);
static void amp_serial_event_cb(lv_event_t *e);
static void wifi_hotspot_event_cb(lv_event_t *e);
static void connect_selected_wifi(void);
static void wifi_keyboard_set_visible(bool visible);
static void wifi_settings_load(void);
static void wifi_settings_save(const char *ssid, const char *password);
static void wifi_connect_to(const char *ssid, const char *password, bool save_on_success);
static void wifi_connect_saved(void);
static void wifi_set_status_text(const char *status_text, const char *startup_text);
static void wifi_apply_ui_updates(void);
static void wifi_apply_scan_results(void);
static void reboot_after_config_change(const char *message);

class WifiStateLock {
public:
    WifiStateLock() { wifi_state_mutex.lock(); }
    ~WifiStateLock() { wifi_state_mutex.unlock(); }

    WifiStateLock(const WifiStateLock &) = delete;
    WifiStateLock &operator=(const WifiStateLock &) = delete;
};

static lv_obj_t *create_wifi_button(lv_obj_t *parent, const char *text, int x, int y, lv_event_cb_t cb) {
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_size(button, 150, 44);
    lv_obj_set_pos(button, x, y);
    lv_obj_add_event_cb(button, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    return button;
}

void wifi_setup_create(void) {
    wifi_settings_load();

    wifi_panel = lv_obj_create(lv_layer_top());
    lv_obj_set_size(wifi_panel, 720, 260);
    lv_obj_center(wifi_panel);
    lv_obj_set_style_bg_color(wifi_panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(wifi_panel, 245, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(wifi_panel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(wifi_panel, lv_color_hex(0x202020), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(wifi_panel, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(wifi_panel, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *title = lv_label_create(wifi_panel);
    lv_label_set_text(title, "Controller setup");
    lv_obj_set_pos(title, 0, 0);

    display_flip_button = lv_button_create(wifi_panel);
    lv_obj_set_size(display_flip_button, 220, 42);
    lv_obj_set_pos(display_flip_button, 0, 30);
    lv_obj_add_event_cb(display_flip_button, display_flip_event_cb, LV_EVENT_CLICKED, NULL);
    display_flip_label = lv_label_create(display_flip_button);
    lv_label_set_text_fmt(display_flip_label, "Flip display: %s", app_config_display_flipped() ? "On" : "Off");
    lv_obj_center(display_flip_label);

    amp_serial_dropdown = lv_dropdown_create(wifi_panel);
    lv_obj_set_size(amp_serial_dropdown, 250, 42);
    lv_obj_set_pos(amp_serial_dropdown, 245, 30);
    lv_dropdown_set_options(amp_serial_dropdown, "Amp serial: UART\nAmp serial: USB");
    lv_dropdown_set_selected(amp_serial_dropdown, app_config_amp_serial_usb() ? 1 : 0);
    lv_obj_add_event_cb(amp_serial_dropdown, amp_serial_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    wifi_dropdown = lv_dropdown_create(wifi_panel);
    lv_obj_set_size(wifi_dropdown, 430, 42);
    lv_obj_set_pos(wifi_dropdown, 0, 78);
    lv_dropdown_set_options(wifi_dropdown, "Not scanned");

    wifi_password = lv_textarea_create(wifi_panel);
    lv_obj_set_size(wifi_password, 250, 42);
    lv_obj_set_pos(wifi_password, 445, 78);
    lv_textarea_set_one_line(wifi_password, true);
    lv_textarea_set_password_mode(wifi_password, true);
    lv_textarea_set_placeholder_text(wifi_password, "Password");
    lv_obj_add_event_cb(wifi_password, wifi_password_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(wifi_password, wifi_password_event_cb, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(wifi_password, wifi_password_event_cb, LV_EVENT_CANCEL, NULL);

    create_wifi_button(wifi_panel, "Connect", 0, 138, wifi_connect_event_cb);
    create_wifi_button(wifi_panel, "Search", 170, 138, wifi_rescan_event_cb);
    create_wifi_button(wifi_panel, "Close", 340, 138, wifi_skip_event_cb);

    wifi_status_label = lv_label_create(wifi_panel);
    lv_obj_set_width(wifi_status_label, 690);
    lv_obj_set_pos(wifi_status_label, 0, 200);
    if (saved_credentials_valid) {
        lv_label_set_text_fmt(wifi_status_label, "Saved network: %s. Auto-connect will run shortly.", saved_ssid);
    } else {
        lv_label_set_text(wifi_status_label, "Tap Search to scan for WiFi networks.");
    }

    wifi_keyboard = lv_keyboard_create(lv_layer_top());
    lv_obj_set_size(wifi_keyboard, 800, 180);
    lv_obj_align(wifi_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(wifi_keyboard, wifi_password);
    lv_obj_add_flag(wifi_keyboard, LV_OBJ_FLAG_HIDDEN);

    wifi_hotspot = lv_obj_create(lv_layer_top());
    lv_obj_set_size(wifi_hotspot, 64, 64);
    lv_obj_set_pos(wifi_hotspot, 0, 0);
    lv_obj_set_style_bg_opa(wifi_hotspot, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(wifi_hotspot, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(wifi_hotspot, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(wifi_hotspot, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(wifi_hotspot, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(wifi_hotspot, wifi_hotspot_event_cb, LV_EVENT_CLICKED, NULL);
}

void wifi_setup_scan_networks(void) {
    bool already_running = false;
    {
        WifiStateLock state_lock;
        if (wifi_scan_running || wifi_scan_requested) {
            already_running = true;
        } else {
            wifi_scan_requested = true;
            wifi_scan_result_pending = false;
            wifi_network_count = 0;
            wifi_dropdown_options[0] = '\0';
        }
    }

    if (already_running) {
        wifi_set_status_text("WiFi scan already running...", "Scanning WiFi networks");
        return;
    }

    if (wifi_dropdown) {
        lv_dropdown_set_options(wifi_dropdown, "Scanning...");
    }
    wifi_set_status_text("Scanning...", "Scanning WiFi networks");
}

void wifi_setup_service(void) {
    bool hide_requested = false;
    bool connecting = false;
    unsigned long connect_started = 0;

    wifi_apply_ui_updates();
    wifi_apply_scan_results();
    {
        WifiStateLock state_lock;
        hide_requested = wifi_hide_requested;
        wifi_hide_requested = false;
        connecting = wifi_connecting;
        connect_started = wifi_connect_started;
    }

    if (hide_requested) {
        wifi_setup_set_visible(false);
    }
    if (connecting) {
        const unsigned long elapsed = millis() - connect_started;
        const int value = 10 + static_cast<int>((elapsed % 3000UL) / 30UL);
        lv_bar_set_value(ui_startupBar, value > 100 ? 100 : value, LV_ANIM_OFF);
    }
}

void wifi_setup_connection_service(void) {
    const unsigned long now = millis();
    bool begin_pending = false;
    bool stack_available = false;
    bool skipped = false;
    bool connected = false;
    bool connecting = false;
    bool saved_valid = false;
    bool scan_requested = false;
    bool save_on_success = false;
    unsigned long connect_started = 0;
    unsigned long next_reconnect = 0;
    char ssid[MAX_WIFI_SSID_LEN + 1];
    char password[MAX_WIFI_PASSWORD_LEN + 1];

    ssid[0] = '\0';
    password[0] = '\0';

    {
        WifiStateLock state_lock;
        stack_available = wifi_stack_available;
        skipped = wifi_skipped;
        begin_pending = wifi_begin_pending;
        connected = wifi_connected;
        connecting = wifi_connecting;
        saved_valid = saved_credentials_valid;
        scan_requested = wifi_scan_requested;
        connect_started = wifi_connect_started;
        next_reconnect = wifi_next_reconnect;
        if (scan_requested) {
            wifi_scan_requested = false;
            wifi_scan_running = true;
            wifi_scan_result_pending = false;
        }
        if (begin_pending) {
            wifi_begin_pending = false;
            save_on_success = pending_save_on_success;
            strncpy(ssid, pending_ssid, sizeof(ssid) - 1);
            ssid[sizeof(ssid) - 1] = '\0';
            strncpy(password, pending_password, sizeof(password) - 1);
            password[sizeof(password) - 1] = '\0';
        }
    }

    if (scan_requested) {
        int networks = WL_IDLE_STATUS;
        int count = 0;
        char scanned_ssids[MAX_WIFI_NETWORKS][MAX_WIFI_SSID_LEN + 1];
        long scanned_rssi[MAX_WIFI_NETWORKS];

        memset(scanned_ssids, 0, sizeof(scanned_ssids));
        memset(scanned_rssi, 0, sizeof(scanned_rssi));

        {
            WifiStackLock lock;
            if (WiFi.status() == WL_NO_MODULE) {
                networks = WL_NO_MODULE;
            } else {
                networks = WiFi.scanNetworks();
                if (networks > 0) {
                    count = networks > MAX_WIFI_NETWORKS ? MAX_WIFI_NETWORKS : networks;
                    for (int i = 0; i < count; ++i) {
                        String ssid_text = WiFi.SSID(i);
                        ssid_text.toCharArray(scanned_ssids[i], MAX_WIFI_SSID_LEN + 1);
                        scanned_rssi[i] = WiFi.RSSI(i);
                    }
                }
            }
        }

        Serial.print("WiFi networks found: ");
        Serial.println(networks);
        for (int i = 0; i < count; ++i) {
            Serial.print(i);
            Serial.print(": ");
            Serial.print(scanned_ssids[i]);
            Serial.print(" RSSI ");
            Serial.println(scanned_rssi[i]);
        }

        {
            WifiStateLock state_lock;
            if (networks == WL_NO_MODULE || networks < 0) {
                wifi_stack_available = false;
                if (networks == WL_NO_MODULE) {
                    wifi_setup_active = false;
                    wifi_connecting = false;
                }
            }
            wifi_scan_result = networks;
            wifi_scan_count = count;
            for (int i = 0; i < count; ++i) {
                strncpy(wifi_scan_ssids[i], scanned_ssids[i], sizeof(wifi_scan_ssids[i]) - 1);
                wifi_scan_ssids[i][sizeof(wifi_scan_ssids[i]) - 1] = '\0';
                wifi_scan_rssi[i] = scanned_rssi[i];
            }
            wifi_scan_running = false;
            wifi_scan_result_pending = true;
        }
        return;
    }

    if (!stack_available || skipped) return;

    int status = WL_IDLE_STATUS;
    {
        WifiStackLock lock;
        status = WiFi.status();
    }

    if (begin_pending) {
        Serial.print("WiFi connecting to ");
        Serial.println(ssid);

        int result = WL_IDLE_STATUS;
        {
            WifiStackLock lock;
            WiFi.setTimeout(WIFI_CONNECT_TIMEOUT_MS);
            result = password[0] ? WiFi.begin(ssid, password) : WiFi.begin(ssid);
        }
        Serial.print("WiFi begin result: ");
        Serial.println(result);

        if (result == WL_CONNECTED) {
            String ip;
            {
                WifiStackLock lock;
                ip = WiFi.localIP().toString();
            }
            if (save_on_success) {
                wifi_settings_save(ssid, password);
            }
            {
                WifiStateLock state_lock;
                wifi_connecting = false;
                wifi_connected = true;
                wifi_setup_active = false;
                pending_save_on_success = false;
                wifi_hide_requested = true;
            }
            char status_text[96];
            char startup_text[96];
            snprintf(status_text, sizeof(status_text), "Connected: %s", ssid);
            snprintf(startup_text, sizeof(startup_text), "WiFi connected: %s", ip.c_str());
            wifi_set_status_text(status_text, startup_text);
            return;
        }

        if (result == WL_NO_MODULE) {
            {
                WifiStateLock state_lock;
                wifi_stack_available = false;
                wifi_connecting = false;
                wifi_connected = false;
                pending_save_on_success = false;
            }
            wifi_set_status_text("WiFi firmware unavailable.", "WiFi unavailable");
            return;
        }

        char status_text[96];
        snprintf(status_text, sizeof(status_text), "Connecting to %s... status %d", ssid, result);
        wifi_set_status_text(status_text, nullptr);
    }

    if (status == WL_CONNECTED) {
        if (!connected) {
            char save_ssid[MAX_WIFI_SSID_LEN + 1];
            char save_password[MAX_WIFI_PASSWORD_LEN + 1];
            bool save_pending = false;
            String connected_ssid;
            String ip;

            save_ssid[0] = '\0';
            save_password[0] = '\0';
            {
                WifiStateLock state_lock;
                save_pending = pending_save_on_success;
                strncpy(save_ssid, pending_ssid, sizeof(save_ssid) - 1);
                save_ssid[sizeof(save_ssid) - 1] = '\0';
                strncpy(save_password, pending_password, sizeof(save_password) - 1);
                save_password[sizeof(save_password) - 1] = '\0';
            }
            {
                WifiStackLock lock;
                connected_ssid = WiFi.SSID();
                ip = WiFi.localIP().toString();
            }
            if (save_pending) {
                wifi_settings_save(save_ssid, save_password);
            }
            {
                WifiStateLock state_lock;
                wifi_connecting = false;
                wifi_setup_active = false;
                wifi_connected = true;
                pending_save_on_success = false;
                wifi_hide_requested = true;
            }
            char status_text[96];
            char startup_text[96];
            snprintf(status_text, sizeof(status_text), "Connected: %s", connected_ssid.c_str());
            snprintf(startup_text, sizeof(startup_text), "WiFi connected: %s", ip.c_str());
            wifi_set_status_text(status_text, startup_text);
        }
        return;
    }

    if (connected) {
        {
            WifiStateLock state_lock;
            wifi_connected = false;
            wifi_setup_active = true;
            wifi_next_reconnect = now;
        }
        Serial.print("WiFi disconnected, status ");
        Serial.println(status);
    }

    if (connecting) {
        if (now - connect_started > WIFI_CONNECT_TIMEOUT_MS) {
            {
                WifiStackLock lock;
                WiFi.disconnect();
            }
            {
                WifiStateLock state_lock;
                wifi_connecting = false;
                wifi_connected = false;
                wifi_next_reconnect = now + WIFI_RECONNECT_RETRY_MS;
            }
            wifi_set_status_text("Connection failed. Retrying in background.", "WiFi retry pending");
        }
        return;
    }

    if (saved_valid && now >= next_reconnect) {
        wifi_connect_saved();
    }
}

bool wifi_setup_is_connected(void) {
    bool connected = false;
    {
        WifiStateLock state_lock;
        connected = wifi_connected;
    }
    if (!connected) {
        return false;
    }
    WifiStackLock lock;
    return WiFi.status() == WL_CONNECTED;
}

bool wifi_setup_has_saved_credentials(void) {
    if (!saved_credentials_loaded) {
        wifi_settings_load();
    }
    WifiStateLock state_lock;
    return saved_credentials_valid;
}

void wifi_setup_clear_saved_credentials(void) {
    app_config_clear_wifi_credentials();

    {
        WifiStateLock state_lock;
        saved_ssid[0] = '\0';
        saved_password[0] = '\0';
        pending_ssid[0] = '\0';
        pending_password[0] = '\0';
        saved_credentials_valid = false;
        saved_credentials_loaded = true;
        wifi_connecting = false;
        wifi_connected = false;
        pending_save_on_success = false;
        wifi_next_reconnect = millis() + WIFI_RECONNECT_RETRY_MS;
    }
    {
        WifiStackLock lock;
        WiFi.disconnect();
    }
    Serial.println("Saved WiFi credentials cleared");
}

void wifi_setup_print_saved_credentials(void) {
    if (!saved_credentials_loaded) {
        wifi_settings_load();
    }

    char ssid[MAX_WIFI_SSID_LEN + 1];
    size_t password_length = 0;
    bool saved_valid = false;
    {
        WifiStateLock state_lock;
        saved_valid = saved_credentials_valid;
        strncpy(ssid, saved_ssid, sizeof(ssid) - 1);
        ssid[sizeof(ssid) - 1] = '\0';
        password_length = strlen(saved_password);
    }

    Serial.print("Saved WiFi credentials=");
    Serial.println(saved_valid ? "yes" : "no");
    if (saved_valid) {
        Serial.print("Saved SSID=");
        Serial.println(ssid);
        Serial.print("Saved password length=");
        Serial.println(password_length);
    }
}

void wifi_setup_set_visible(bool visible) {
    if (!wifi_panel) return;
    if (visible) {
        lv_obj_move_foreground(wifi_panel);
        if (wifi_hotspot) lv_obj_move_foreground(wifi_hotspot);
        lv_obj_remove_flag(wifi_panel, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(wifi_panel, LV_OBJ_FLAG_HIDDEN);
        wifi_keyboard_set_visible(false);
    }
}

static void connect_selected_wifi(void) {
    bool stack_available = false;
    {
        WifiStateLock state_lock;
        stack_available = wifi_stack_available;
    }

    if (wifi_network_count <= 0) {
        lv_label_set_text(wifi_status_label, stack_available ? "No network selected. Search first." : "WiFi unavailable. Amplifier control will continue.");
        return;
    }

    uint16_t selected = lv_dropdown_get_selected(wifi_dropdown);
    if (selected >= wifi_network_count) selected = 0;

    const char *password = lv_textarea_get_text(wifi_password);
    wifi_keyboard_set_visible(false);
    wifi_connect_to(wifi_ssids[selected], password, true);
}

static void wifi_connect_event_cb(lv_event_t *e) {
    (void)e;
    connect_selected_wifi();
}

static void wifi_rescan_event_cb(lv_event_t *e) {
    (void)e;
    wifi_setup_scan_networks();
}

static void wifi_skip_event_cb(lv_event_t *e) {
    (void)e;
    wifi_setup_set_visible(false);
    Serial.println("WiFi setup closed; background reconnect remains active");
}

static void wifi_password_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(wifi_keyboard, wifi_password);
        wifi_keyboard_set_visible(true);
    } else if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        wifi_keyboard_set_visible(false);
    }
}

static void display_flip_event_cb(lv_event_t *e) {
    (void)e;
    const bool flipped = !app_config_display_flipped();
    giga_lvgl_display_set_flipped(flipped);
    const bool saved = app_config_set_display_flipped(flipped);
    if (display_flip_label) {
        lv_label_set_text_fmt(display_flip_label, "Flip display: %s", flipped ? "On" : "Off");
        lv_obj_center(display_flip_label);
    }
    lv_label_set_text(wifi_status_label, saved ? "Display orientation updated." : "Display orientation changed, but saving failed.");
}

static void amp_serial_event_cb(lv_event_t *e) {
    lv_obj_t *dropdown = static_cast<lv_obj_t *>(lv_event_get_target(e));
    const bool enabled = lv_dropdown_get_selected(dropdown) == 1;
    const bool current = app_config_amp_serial_usb();

    if (enabled == current) {
        return;
    }

    if (!app_config_set_amp_serial_usb(enabled)) {
        lv_dropdown_set_selected(dropdown, current ? 1 : 0);
        lv_label_set_text(wifi_status_label, "Amp serial setting changed, but saving failed.");
        return;
    }

    reboot_after_config_change(enabled ? "USB amp serial saved. Rebooting..." : "UART amp serial saved. Rebooting...");
}

static void wifi_hotspot_event_cb(lv_event_t *e) {
    (void)e;
    {
        WifiStateLock state_lock;
        wifi_skipped = false;
        wifi_setup_active = true;
    }
    wifi_setup_set_visible(true);
}

static void wifi_keyboard_set_visible(bool visible) {
    if (!wifi_keyboard) return;

    if (visible) {
        if (wifi_panel) {
            lv_obj_align(wifi_panel, LV_ALIGN_TOP_MID, 0, 12);
            lv_obj_move_foreground(wifi_panel);
        }
        lv_obj_move_foreground(wifi_keyboard);
        lv_obj_remove_flag(wifi_keyboard, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(wifi_keyboard, LV_OBJ_FLAG_HIDDEN);
        if (wifi_panel) {
            lv_obj_center(wifi_panel);
        }
    }

    if (wifi_hotspot) {
        lv_obj_move_foreground(wifi_hotspot);
    }
}

static void wifi_settings_load(void) {
    app_config_load();
    AppConfig loaded = app_config_snapshot();

    const bool loaded_valid = loaded.wifi_ssid[0] != '\0';
    {
        WifiStateLock state_lock;
        strncpy(saved_ssid, loaded.wifi_ssid, sizeof(saved_ssid) - 1);
        saved_ssid[sizeof(saved_ssid) - 1] = '\0';
        strncpy(saved_password, loaded.wifi_password, sizeof(saved_password) - 1);
        saved_password[sizeof(saved_password) - 1] = '\0';
        saved_credentials_loaded = true;
        saved_credentials_valid = loaded_valid;
    }
    if (loaded_valid) {
        Serial.print("Loaded saved WiFi SSID: ");
        Serial.println(loaded.wifi_ssid);
    } else {
        Serial.println("No saved WiFi credentials");
    }
}

static void wifi_settings_save(const char *ssid, const char *password) {
    if (!ssid || ssid[0] == '\0') {
        return;
    }

    if (app_config_set_wifi_credentials(ssid, password)) {
        {
            WifiStateLock state_lock;
            strncpy(saved_ssid, ssid, sizeof(saved_ssid) - 1);
            saved_ssid[sizeof(saved_ssid) - 1] = '\0';
            strncpy(saved_password, password ? password : "", sizeof(saved_password) - 1);
            saved_password[sizeof(saved_password) - 1] = '\0';
            saved_credentials_valid = true;
            saved_credentials_loaded = true;
        }
        Serial.print("Saved WiFi credentials for SSID: ");
        Serial.println(ssid);
    } else {
        Serial.println("WiFi settings: failed to save settings");
    }
}

static void wifi_connect_to(const char *ssid, const char *password, bool save_on_success) {
    if (!ssid || ssid[0] == '\0') {
        wifi_set_status_text("No SSID selected.", nullptr);
        return;
    }

    {
        WifiStateLock state_lock;
        strncpy(pending_ssid, ssid, sizeof(pending_ssid) - 1);
        pending_ssid[sizeof(pending_ssid) - 1] = '\0';
        strncpy(pending_password, password ? password : "", sizeof(pending_password) - 1);
        pending_password[sizeof(pending_password) - 1] = '\0';
        pending_save_on_success = save_on_success;
        wifi_connecting = true;
        wifi_begin_pending = true;
        wifi_connect_started = millis();
        wifi_next_reconnect = wifi_connect_started + WIFI_CONNECT_TIMEOUT_MS + WIFI_RECONNECT_RETRY_MS;
    }

    char status_text[96];
    char startup_text[96];
    snprintf(status_text, sizeof(status_text), "Connecting to %s...", ssid);
    snprintf(startup_text, sizeof(startup_text), "Connecting to %s", ssid);
    wifi_set_status_text(status_text, startup_text);

}

static void wifi_connect_saved(void) {
    char ssid[MAX_WIFI_SSID_LEN + 1];
    char password[MAX_WIFI_PASSWORD_LEN + 1];
    bool saved_valid = false;
    {
        WifiStateLock state_lock;
        saved_valid = saved_credentials_valid;
        strncpy(ssid, saved_ssid, sizeof(ssid) - 1);
        ssid[sizeof(ssid) - 1] = '\0';
        strncpy(password, saved_password, sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0';
    }
    if (!saved_valid) {
        return;
    }

    Serial.print("Auto-connecting saved WiFi SSID: ");
    Serial.println(ssid);
    wifi_connect_to(ssid, password, false);
}

static void wifi_set_status_text(const char *status_text, const char *startup_text) {
    WifiStateLock state_lock;
    if (status_text) {
        strncpy(wifi_status_text, status_text, sizeof(wifi_status_text) - 1);
        wifi_status_text[sizeof(wifi_status_text) - 1] = '\0';
    }
    if (startup_text) {
        strncpy(wifi_startup_text, startup_text, sizeof(wifi_startup_text) - 1);
        wifi_startup_text[sizeof(wifi_startup_text) - 1] = '\0';
    }
    wifi_ui_dirty = true;
}

static void wifi_apply_ui_updates(void) {
    char status_text[sizeof(wifi_status_text)];
    char startup_text[sizeof(wifi_startup_text)];
    bool dirty = false;

    status_text[0] = '\0';
    startup_text[0] = '\0';
    {
        WifiStateLock state_lock;
        dirty = wifi_ui_dirty;
        if (dirty) {
            strncpy(status_text, wifi_status_text, sizeof(status_text) - 1);
            status_text[sizeof(status_text) - 1] = '\0';
            strncpy(startup_text, wifi_startup_text, sizeof(startup_text) - 1);
            startup_text[sizeof(startup_text) - 1] = '\0';
            wifi_ui_dirty = false;
        }
    }

    if (!dirty) {
        return;
    }
    if (wifi_status_label && status_text[0]) {
        lv_label_set_text(wifi_status_label, status_text);
    }
    if (startup_text[0]) {
        lv_label_set_text(ui_startupMessage, startup_text);
    }
}

static void wifi_apply_scan_results(void) {
    int result = 0;
    int count = 0;
    char scanned_ssids[MAX_WIFI_NETWORKS][MAX_WIFI_SSID_LEN + 1];
    long scanned_rssi[MAX_WIFI_NETWORKS];
    bool pending = false;

    memset(scanned_ssids, 0, sizeof(scanned_ssids));
    memset(scanned_rssi, 0, sizeof(scanned_rssi));

    {
        WifiStateLock state_lock;
        pending = wifi_scan_result_pending;
        if (pending) {
            result = wifi_scan_result;
            count = wifi_scan_count;
            for (int i = 0; i < count; ++i) {
                strncpy(scanned_ssids[i], wifi_scan_ssids[i], sizeof(scanned_ssids[i]) - 1);
                scanned_ssids[i][sizeof(scanned_ssids[i]) - 1] = '\0';
                scanned_rssi[i] = wifi_scan_rssi[i];
            }
            wifi_scan_result_pending = false;
        }
    }

    if (!pending) {
        return;
    }

    wifi_network_count = 0;
    wifi_dropdown_options[0] = '\0';

    if (result == WL_NO_MODULE) {
        if (wifi_dropdown) {
            lv_dropdown_set_options(wifi_dropdown, "WiFi unavailable");
        }
        if (wifi_status_label) {
            lv_label_set_text(wifi_status_label, "WiFi firmware unavailable. Amplifier control will continue.");
        }
        return;
    }

    if (result <= 0 || count <= 0) {
        if (wifi_dropdown) {
            lv_dropdown_set_options(wifi_dropdown, "No networks found");
        }
        if (wifi_status_label) {
            lv_label_set_text(wifi_status_label, result < 0 ? "WiFi scan failed. Amplifier control will continue." : "No networks found. Check antenna/range or search again.");
        }
        return;
    }

    for (int i = 0; i < count; ++i) {
        strncpy(wifi_ssids[i], scanned_ssids[i], sizeof(wifi_ssids[i]) - 1);
        wifi_ssids[i][sizeof(wifi_ssids[i]) - 1] = '\0';

        char line[MAX_WIFI_SSID_LEN + 18];
        snprintf(line, sizeof(line), "%s (%ld dBm)", wifi_ssids[i], scanned_rssi[i]);
        if (i > 0) {
            strncat(wifi_dropdown_options, "\n", sizeof(wifi_dropdown_options) - strlen(wifi_dropdown_options) - 1);
        }
        strncat(wifi_dropdown_options, line, sizeof(wifi_dropdown_options) - strlen(wifi_dropdown_options) - 1);
    }

    wifi_network_count = count;
    if (wifi_dropdown) {
        lv_dropdown_set_options(wifi_dropdown, wifi_dropdown_options);
    }
    if (wifi_status_label) {
        lv_label_set_text(wifi_status_label, "Select a network, enter password, then connect.");
    }
    lv_label_set_text(ui_startupMessage, "WiFi setup ready");
}

static void reboot_after_config_change(const char *message) {
    if (wifi_status_label) {
        lv_label_set_text(wifi_status_label, message);
    }
    lv_timer_handler();
    Serial.println(message);
    Serial.flush();
    delay(250);
    NVIC_SystemReset();
}
