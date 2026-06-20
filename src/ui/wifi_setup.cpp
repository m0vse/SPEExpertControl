/*
 * Hidden WiFi setup popup, saved credential storage, scanning, and background reconnect handling.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "ui/wifi_setup.h"

#include <Arduino.h>
#include <BlockDevice.h>
#include <FATFileSystem.h>
#include <MBRBlockDevice.h>
#include <WiFi.h>
#include <stdio.h>
#include <string.h>
#include <ui.h>

static const int MAX_WIFI_NETWORKS = 12;
static const int MAX_WIFI_SSID_LEN = 32;
static const int MAX_WIFI_PASSWORD_LEN = 64;
static const unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;
static const unsigned long WIFI_AUTOCONNECT_DELAY_MS = 2500;
static const unsigned long WIFI_RECONNECT_RETRY_MS = 30000;
static const char *WIFI_SETTINGS_PATH = "/fs/spe_wifi.cfg";

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
static unsigned long wifi_connect_started = 0;
static unsigned long wifi_next_reconnect = WIFI_AUTOCONNECT_DELAY_MS;
static lv_obj_t *wifi_panel = NULL;
static lv_obj_t *wifi_dropdown = NULL;
static lv_obj_t *wifi_password = NULL;
static lv_obj_t *wifi_keyboard = NULL;
static lv_obj_t *wifi_status_label = NULL;
static lv_obj_t *wifi_hotspot = NULL;

static void wifi_connect_event_cb(lv_event_t *e);
static void wifi_rescan_event_cb(lv_event_t *e);
static void wifi_skip_event_cb(lv_event_t *e);
static void wifi_password_event_cb(lv_event_t *e);
static void wifi_hotspot_event_cb(lv_event_t *e);
static void connect_selected_wifi(void);
static void wifi_keyboard_set_visible(bool visible);
static bool wifi_storage_mount(mbed::BlockDevice *&root, mbed::MBRBlockDevice *&settings_partition, mbed::FATFileSystem *&settings_fs);
static void wifi_storage_unmount(mbed::BlockDevice *root, mbed::FATFileSystem *settings_fs);
static void wifi_settings_load(void);
static void wifi_settings_save(const char *ssid, const char *password);
static void wifi_connect_to(const char *ssid, const char *password, bool save_on_success);
static void wifi_connect_saved(void);

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
    lv_obj_set_size(wifi_panel, 720, 220);
    lv_obj_center(wifi_panel);
    lv_obj_set_style_bg_color(wifi_panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(wifi_panel, 245, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(wifi_panel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(wifi_panel, lv_color_hex(0x202020), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(wifi_panel, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(wifi_panel, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *title = lv_label_create(wifi_panel);
    lv_label_set_text(title, "WiFi setup");
    lv_obj_set_pos(title, 0, 0);

    wifi_dropdown = lv_dropdown_create(wifi_panel);
    lv_obj_set_size(wifi_dropdown, 430, 42);
    lv_obj_set_pos(wifi_dropdown, 0, 38);
    lv_dropdown_set_options(wifi_dropdown, "Not scanned");

    wifi_password = lv_textarea_create(wifi_panel);
    lv_obj_set_size(wifi_password, 250, 42);
    lv_obj_set_pos(wifi_password, 445, 38);
    lv_textarea_set_one_line(wifi_password, true);
    lv_textarea_set_password_mode(wifi_password, true);
    lv_textarea_set_placeholder_text(wifi_password, "Password");
    lv_obj_add_event_cb(wifi_password, wifi_password_event_cb, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(wifi_password, wifi_password_event_cb, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(wifi_password, wifi_password_event_cb, LV_EVENT_CANCEL, NULL);

    create_wifi_button(wifi_panel, "Connect", 0, 98, wifi_connect_event_cb);
    create_wifi_button(wifi_panel, "Search", 170, 98, wifi_rescan_event_cb);
    create_wifi_button(wifi_panel, "Close", 340, 98, wifi_skip_event_cb);

    wifi_status_label = lv_label_create(wifi_panel);
    lv_obj_set_width(wifi_status_label, 690);
    lv_obj_set_pos(wifi_status_label, 0, 160);
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
    wifi_network_count = 0;
    wifi_dropdown_options[0] = '\0';

    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with WiFi module failed!");
        wifi_stack_available = false;
        wifi_setup_active = false;
        wifi_connecting = false;
        lv_label_set_text(wifi_status_label, "WiFi firmware unavailable. Amplifier control will continue.");
        lv_dropdown_set_options(wifi_dropdown, "WiFi unavailable");
        return;
    }

    lv_label_set_text(ui_startupMessage, "Scanning WiFi networks");
    lv_label_set_text(wifi_status_label, "Scanning...");
    lv_timer_handler();

    int networks = WiFi.scanNetworks();
    Serial.print("WiFi networks found: ");
    Serial.println(networks);

    if (networks <= 0) {
        if (networks < 0) {
            wifi_stack_available = false;
        }
        lv_dropdown_set_options(wifi_dropdown, "No networks found");
        lv_label_set_text(wifi_status_label, networks < 0 ? "WiFi scan failed. Amplifier control will continue." : "No networks found. Check antenna/range or search again.");
        return;
    }

    int count = networks;
    if (count > MAX_WIFI_NETWORKS) count = MAX_WIFI_NETWORKS;
    for (int i = 0; i < count; ++i) {
        String ssid = WiFi.SSID(i);
        ssid.toCharArray(wifi_ssids[i], MAX_WIFI_SSID_LEN + 1);

        char line[MAX_WIFI_SSID_LEN + 18];
        snprintf(line, sizeof(line), "%s (%ld dBm)", wifi_ssids[i], WiFi.RSSI(i));
        if (i > 0) strncat(wifi_dropdown_options, "\n", sizeof(wifi_dropdown_options) - strlen(wifi_dropdown_options) - 1);
        strncat(wifi_dropdown_options, line, sizeof(wifi_dropdown_options) - strlen(wifi_dropdown_options) - 1);

        Serial.print(i);
        Serial.print(": ");
        Serial.print(wifi_ssids[i]);
        Serial.print(" RSSI ");
        Serial.println(WiFi.RSSI(i));
    }

    wifi_network_count = count;
    lv_dropdown_set_options(wifi_dropdown, wifi_dropdown_options);
    lv_label_set_text(wifi_status_label, "Select a network, enter password, then connect.");
    lv_label_set_text(ui_startupMessage, "WiFi setup ready");
}

void wifi_setup_service(void) {
    if (!wifi_stack_available || wifi_skipped) return;

    const unsigned long now = millis();
    const int status = WiFi.status();

    if (status == WL_CONNECTED) {
        if (!wifi_connected) {
            wifi_connecting = false;
            wifi_setup_active = false;
            wifi_connected = true;
            String connected_ssid = WiFi.SSID();
            String ip = WiFi.localIP().toString();
            if (pending_save_on_success) {
                wifi_settings_save(pending_ssid, pending_password);
                pending_save_on_success = false;
            }
            if (wifi_status_label) {
                lv_label_set_text_fmt(wifi_status_label, "Connected: %s", connected_ssid.c_str());
            }
            lv_label_set_text_fmt(ui_startupMessage, "WiFi connected: %s", ip.c_str());
            wifi_setup_set_visible(false);
        }
        return;
    }

    if (wifi_connected) {
        wifi_connected = false;
        wifi_setup_active = true;
        wifi_next_reconnect = now;
        Serial.print("WiFi disconnected, status ");
        Serial.println(status);
    }

    if (wifi_connecting) {
        if (now - wifi_connect_started > WIFI_CONNECT_TIMEOUT_MS) {
            wifi_connecting = false;
            wifi_connected = false;
            WiFi.disconnect();
            wifi_next_reconnect = now + WIFI_RECONNECT_RETRY_MS;
            if (wifi_status_label) {
                lv_label_set_text(wifi_status_label, "Connection failed. Retrying in background.");
            }
            lv_label_set_text(ui_startupMessage, "WiFi retry pending");
        }
        return;
    }

    if (saved_credentials_valid && now >= wifi_next_reconnect) {
        wifi_connect_saved();
    }
}

bool wifi_setup_is_connected(void) {
    return wifi_connected && WiFi.status() == WL_CONNECTED;
}

bool wifi_setup_has_saved_credentials(void) {
    if (!saved_credentials_loaded) {
        wifi_settings_load();
    }
    return saved_credentials_valid;
}

void wifi_setup_clear_saved_credentials(void) {
    mbed::BlockDevice *root = nullptr;
    mbed::MBRBlockDevice *settings_partition = nullptr;
    mbed::FATFileSystem *settings_fs = nullptr;

    if (wifi_storage_mount(root, settings_partition, settings_fs)) {
        remove(WIFI_SETTINGS_PATH);
        wifi_storage_unmount(root, settings_fs);
        delete settings_fs;
        delete settings_partition;
    }

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
    WiFi.disconnect();
    Serial.println("Saved WiFi credentials cleared");
}

void wifi_setup_print_saved_credentials(void) {
    if (!saved_credentials_loaded) {
        wifi_settings_load();
    }

    Serial.print("Saved WiFi credentials=");
    Serial.println(saved_credentials_valid ? "yes" : "no");
    if (saved_credentials_valid) {
        Serial.print("Saved SSID=");
        Serial.println(saved_ssid);
        Serial.print("Saved password length=");
        Serial.println(strlen(saved_password));
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
    if (wifi_network_count <= 0) {
        lv_label_set_text(wifi_status_label, wifi_stack_available ? "No network selected. Search first." : "WiFi unavailable. Amplifier control will continue.");
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

static void wifi_hotspot_event_cb(lv_event_t *e) {
    (void)e;
    wifi_skipped = false;
    wifi_setup_active = true;
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

static bool wifi_storage_mount(mbed::BlockDevice *&root, mbed::MBRBlockDevice *&settings_partition, mbed::FATFileSystem *&settings_fs) {
    root = mbed::BlockDevice::get_default_instance();
    if (!root) {
        Serial.println("WiFi settings: no default block device");
        return false;
    }

    int err = root->init();
    if (err) {
        Serial.print("WiFi settings: QSPI init failed ");
        Serial.println(err);
        return false;
    }

    settings_partition = new mbed::MBRBlockDevice(root, 2);
    settings_fs = new mbed::FATFileSystem("fs");

    err = settings_fs->mount(settings_partition);
    if (err) {
        Serial.println("WiFi settings: creating QSPI partition 2");
        mbed::MBRBlockDevice::partition(root, 2, 0x0B, 1 * 1024 * 1024, 6 * 1024 * 1024);
        err = settings_fs->reformat(settings_partition);
    }

    if (err) {
        Serial.print("WiFi settings: filesystem mount failed ");
        Serial.println(err);
        delete settings_fs;
        delete settings_partition;
        settings_fs = nullptr;
        settings_partition = nullptr;
        root->deinit();
        return false;
    }

    return true;
}

static void wifi_storage_unmount(mbed::BlockDevice *root, mbed::FATFileSystem *settings_fs) {
    if (settings_fs) {
        settings_fs->unmount();
    }
    if (root) {
        root->deinit();
    }
}

static void wifi_settings_load(void) {
    saved_credentials_loaded = true;
    saved_credentials_valid = false;
    saved_ssid[0] = '\0';
    saved_password[0] = '\0';

    mbed::BlockDevice *root = nullptr;
    mbed::MBRBlockDevice *settings_partition = nullptr;
    mbed::FATFileSystem *settings_fs = nullptr;

    if (!wifi_storage_mount(root, settings_partition, settings_fs)) {
        return;
    }

    FILE *fp = fopen(WIFI_SETTINGS_PATH, "r");
    if (fp) {
        char line[128];
        while (fgets(line, sizeof(line), fp)) {
            char *newline = strchr(line, '\n');
            if (newline) *newline = '\0';
            char *cr = strchr(line, '\r');
            if (cr) *cr = '\0';

            if (strncmp(line, "ssid=", 5) == 0) {
                strncpy(saved_ssid, line + 5, sizeof(saved_ssid) - 1);
                saved_ssid[sizeof(saved_ssid) - 1] = '\0';
            } else if (strncmp(line, "password=", 9) == 0) {
                strncpy(saved_password, line + 9, sizeof(saved_password) - 1);
                saved_password[sizeof(saved_password) - 1] = '\0';
            }
        }
        fclose(fp);
    }

    wifi_storage_unmount(root, settings_fs);
    delete settings_fs;
    delete settings_partition;

    saved_credentials_valid = saved_ssid[0] != '\0';
    if (saved_credentials_valid) {
        Serial.print("Loaded saved WiFi SSID: ");
        Serial.println(saved_ssid);
    } else {
        Serial.println("No saved WiFi credentials");
    }
}

static void wifi_settings_save(const char *ssid, const char *password) {
    if (!ssid || ssid[0] == '\0') {
        return;
    }

    mbed::BlockDevice *root = nullptr;
    mbed::MBRBlockDevice *settings_partition = nullptr;
    mbed::FATFileSystem *settings_fs = nullptr;

    if (!wifi_storage_mount(root, settings_partition, settings_fs)) {
        return;
    }

    FILE *fp = fopen(WIFI_SETTINGS_PATH, "w");
    if (!fp) {
        Serial.println("WiFi settings: failed to open settings file for write");
    } else {
        fprintf(fp, "ssid=%s\n", ssid);
        fprintf(fp, "password=%s\n", password ? password : "");
        fclose(fp);
        strncpy(saved_ssid, ssid, sizeof(saved_ssid) - 1);
        saved_ssid[sizeof(saved_ssid) - 1] = '\0';
        strncpy(saved_password, password ? password : "", sizeof(saved_password) - 1);
        saved_password[sizeof(saved_password) - 1] = '\0';
        saved_credentials_valid = true;
        saved_credentials_loaded = true;
        Serial.print("Saved WiFi credentials for SSID: ");
        Serial.println(saved_ssid);
    }

    wifi_storage_unmount(root, settings_fs);
    delete settings_fs;
    delete settings_partition;
}

static void wifi_connect_to(const char *ssid, const char *password, bool save_on_success) {
    if (!ssid || ssid[0] == '\0') {
        lv_label_set_text(wifi_status_label, "No SSID selected.");
        return;
    }

    strncpy(pending_ssid, ssid, sizeof(pending_ssid) - 1);
    pending_ssid[sizeof(pending_ssid) - 1] = '\0';
    strncpy(pending_password, password ? password : "", sizeof(pending_password) - 1);
    pending_password[sizeof(pending_password) - 1] = '\0';
    pending_save_on_success = save_on_success;

    lv_label_set_text_fmt(wifi_status_label, "Connecting to %s...", pending_ssid);
    lv_label_set_text_fmt(ui_startupMessage, "Connecting to %s", pending_ssid);
    lv_timer_handler();

    Serial.print("WiFi connecting to ");
    Serial.println(pending_ssid);

    WiFi.setTimeout(WIFI_CONNECT_TIMEOUT_MS);
    int result = pending_password[0] ? WiFi.begin(pending_ssid, pending_password) : WiFi.begin(pending_ssid);
    Serial.print("WiFi begin result: ");
    Serial.println(result);

    if (result == WL_CONNECTED) {
        wifi_connecting = false;
        wifi_connected = true;
        wifi_setup_active = false;
        String ip = WiFi.localIP().toString();
        if (save_on_success) {
            wifi_settings_save(pending_ssid, pending_password);
        }
        pending_save_on_success = false;
        lv_label_set_text_fmt(wifi_status_label, "Connected: %s", pending_ssid);
        lv_label_set_text_fmt(ui_startupMessage, "WiFi connected: %s", ip.c_str());
        wifi_setup_set_visible(false);
        return;
    }

    if (result == WL_NO_MODULE) {
        wifi_stack_available = false;
        wifi_connecting = false;
        wifi_connected = false;
        pending_save_on_success = false;
        lv_label_set_text(wifi_status_label, "WiFi firmware unavailable.");
        lv_label_set_text(ui_startupMessage, "WiFi unavailable");
        return;
    }

    wifi_connecting = true;
    wifi_connect_started = millis();
    wifi_next_reconnect = wifi_connect_started + WIFI_CONNECT_TIMEOUT_MS + WIFI_RECONNECT_RETRY_MS;
    lv_label_set_text_fmt(wifi_status_label, "Connecting to %s... status %d", pending_ssid, result);
}

static void wifi_connect_saved(void) {
    if (!saved_credentials_valid) {
        return;
    }
    Serial.print("Auto-connecting saved WiFi SSID: ");
    Serial.println(saved_ssid);
    wifi_connect_to(saved_ssid, saved_password, false);
}
