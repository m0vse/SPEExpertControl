/*
 * Persistent controller configuration stored on the Giga QSPI filesystem.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "app_config.h"

#include <BlockDevice.h>
#include <FATFileSystem.h>
#include <MBRBlockDevice.h>
#include <rtos.h>
#include <stdio.h>
#include <string.h>

static const char *APP_CONFIG_PATH = "/fs/spe_wifi.cfg";

static AppConfig config;
static bool config_loaded = false;
static rtos::Mutex config_mutex;

class ConfigLock {
public:
    ConfigLock() { config_mutex.lock(); }
    ~ConfigLock() { config_mutex.unlock(); }
};

static bool config_storage_mount(mbed::BlockDevice *&root, mbed::MBRBlockDevice *&settings_partition, mbed::FATFileSystem *&settings_fs)
{
    root = mbed::BlockDevice::get_default_instance();
    if (!root) {
        Serial.println("Config: no default block device");
        return false;
    }

    int err = root->init();
    if (err) {
        Serial.print("Config: QSPI init failed ");
        Serial.println(err);
        return false;
    }

    settings_partition = new mbed::MBRBlockDevice(root, 2);
    settings_fs = new mbed::FATFileSystem("fs");

    err = settings_fs->mount(settings_partition);
    if (err) {
        Serial.println("Config: creating QSPI partition 2");
        mbed::MBRBlockDevice::partition(root, 2, 0x0B, 1 * 1024 * 1024, 6 * 1024 * 1024);
        err = settings_fs->reformat(settings_partition);
    }

    if (err) {
        Serial.print("Config: filesystem mount failed ");
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

static void config_storage_unmount(mbed::BlockDevice *root, mbed::FATFileSystem *settings_fs)
{
    if (settings_fs) {
        settings_fs->unmount();
    }
    if (root) {
        root->deinit();
    }
}

bool app_config_load(void)
{
    AppConfig loaded;
    mbed::BlockDevice *root = nullptr;
    mbed::MBRBlockDevice *settings_partition = nullptr;
    mbed::FATFileSystem *settings_fs = nullptr;

    if (!config_storage_mount(root, settings_partition, settings_fs)) {
        return false;
    }

    FILE *fp = fopen(APP_CONFIG_PATH, "r");
    if (fp) {
        char line[128];
        while (fgets(line, sizeof(line), fp)) {
            char *newline = strchr(line, '\n');
            if (newline) *newline = '\0';
            char *cr = strchr(line, '\r');
            if (cr) *cr = '\0';

            if (strncmp(line, "ssid=", 5) == 0) {
                strncpy(loaded.wifi_ssid, line + 5, sizeof(loaded.wifi_ssid) - 1);
                loaded.wifi_ssid[sizeof(loaded.wifi_ssid) - 1] = '\0';
            } else if (strncmp(line, "password=", 9) == 0) {
                strncpy(loaded.wifi_password, line + 9, sizeof(loaded.wifi_password) - 1);
                loaded.wifi_password[sizeof(loaded.wifi_password) - 1] = '\0';
            } else if (strncmp(line, "display_flipped=", 16) == 0) {
                loaded.display_flipped = strcmp(line + 16, "1") == 0 || strcmp(line + 16, "true") == 0;
            } else if (strncmp(line, "amp_serial_usb=", 15) == 0) {
                loaded.amp_serial_usb = strcmp(line + 15, "1") == 0 || strcmp(line + 15, "true") == 0;
            }
        }
        fclose(fp);
    }

    config_storage_unmount(root, settings_fs);
    delete settings_fs;
    delete settings_partition;

    {
        ConfigLock lock;
        config = loaded;
        config_loaded = true;
    }

    return true;
}

bool app_config_save(void)
{
    AppConfig snapshot = app_config_snapshot();
    mbed::BlockDevice *root = nullptr;
    mbed::MBRBlockDevice *settings_partition = nullptr;
    mbed::FATFileSystem *settings_fs = nullptr;

    if (!config_storage_mount(root, settings_partition, settings_fs)) {
        return false;
    }

    bool saved = false;
    FILE *fp = fopen(APP_CONFIG_PATH, "w");
    if (!fp) {
        Serial.println("Config: failed to open settings file for write");
    } else {
        fprintf(fp, "ssid=%s\n", snapshot.wifi_ssid);
        fprintf(fp, "password=%s\n", snapshot.wifi_password);
        fprintf(fp, "display_flipped=%d\n", snapshot.display_flipped ? 1 : 0);
        fprintf(fp, "amp_serial_usb=%d\n", snapshot.amp_serial_usb ? 1 : 0);
        fclose(fp);
        saved = true;
    }

    config_storage_unmount(root, settings_fs);
    delete settings_fs;
    delete settings_partition;
    return saved;
}

AppConfig app_config_snapshot(void)
{
    ConfigLock lock;
    return config;
}

bool app_config_display_flipped(void)
{
    ConfigLock lock;
    return config.display_flipped;
}

bool app_config_set_display_flipped(bool flipped)
{
    {
        ConfigLock lock;
        config.display_flipped = flipped;
        config_loaded = true;
    }
    return app_config_save();
}

bool app_config_amp_serial_usb(void)
{
    ConfigLock lock;
    return config.amp_serial_usb;
}

bool app_config_set_amp_serial_usb(bool enabled)
{
    {
        ConfigLock lock;
        config.amp_serial_usb = enabled;
        config_loaded = true;
    }
    return app_config_save();
}

bool app_config_set_wifi_credentials(const char *ssid, const char *password)
{
    if (!ssid || ssid[0] == '\0') {
        return false;
    }

    {
        ConfigLock lock;
        strncpy(config.wifi_ssid, ssid, sizeof(config.wifi_ssid) - 1);
        config.wifi_ssid[sizeof(config.wifi_ssid) - 1] = '\0';
        strncpy(config.wifi_password, password ? password : "", sizeof(config.wifi_password) - 1);
        config.wifi_password[sizeof(config.wifi_password) - 1] = '\0';
        config_loaded = true;
    }
    return app_config_save();
}

bool app_config_clear_wifi_credentials(void)
{
    {
        ConfigLock lock;
        config.wifi_ssid[0] = '\0';
        config.wifi_password[0] = '\0';
        config_loaded = true;
    }
    return app_config_save();
}
