/*
 * Standalone PlatformIO firmware used to install or refresh the Arduino Giga WiFi firmware partition.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include <Arduino.h>

#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FATFileSystem.h"
#include "resources.h"
#include "certificates.h"

#ifndef CORE_CM7
#error Update the WiFi firmware by uploading this firmware to the M7 core.
#endif

QSPIFBlockDevice root(QSPI_SO0, QSPI_SO1, QSPI_SO2, QSPI_SO3, QSPI_SCK, QSPI_CS, QSPIF_POLARITY_MODE_1, 40000000);
mbed::MBRBlockDevice wifi_data(&root, 1);
mbed::FATFileSystem wifi_data_fs("wlan");

static void printProgress(uint32_t offset, uint32_t size, uint32_t threshold, bool reset)
{
    static int percent_done = 0;
    if (reset) {
        percent_done = 0;
        Serial.println("Flashed " + String(percent_done) + "%");
        return;
    }

    uint32_t percent_done_new = offset * 100 / size;
    if (percent_done_new >= static_cast<uint32_t>(percent_done + threshold)) {
        percent_done = percent_done_new;
        Serial.println("Flashed " + String(percent_done) + "%");
    }
}

static void writeFirmwareFile()
{
    FILE *fp = fopen("/wlan/4343WA1.BIN", "wb");
    if (!fp) {
        Serial.println("Error opening /wlan/4343WA1.BIN");
        return;
    }

    const int file_size = 421098;
    int chunk_size = 1024;
    int byte_count = 0;

    Serial.println("Flashing /wlan/4343WA1.BIN file");
    printProgress(byte_count, file_size, 10, true);
    while (byte_count < file_size) {
        if (byte_count + chunk_size > file_size) {
            chunk_size = file_size - byte_count;
        }
        int ret = fwrite(&wifi_firmware_image_data[byte_count], chunk_size, 1, fp);
        if (ret != 1) {
            Serial.println("Error writing firmware data");
            break;
        }
        byte_count += chunk_size;
        printProgress(byte_count, file_size, 10, false);
    }
    fclose(fp);
}

static void writeMemoryMappedFirmware()
{
    const int file_size = 421098;
    int chunk_size = 1024;
    int byte_count = 0;
    const uint32_t offset = 15 * 1024 * 1024 + 1024 * 512;

    Serial.println("Erasing memory mapped firmware area...");
    int err = root.erase(14 * 1024 * 1024, 2 * 1024 * 1024);
    if (err != 0) {
        Serial.println("Error erasing memory mapped firmware area");
    }

    Serial.println("Flashing memory mapped firmware");
    printProgress(byte_count, file_size, 10, true);
    while (byte_count < file_size) {
        if (byte_count + chunk_size > file_size) {
            chunk_size = file_size - byte_count;
        }
        int ret = root.program(&wifi_firmware_image_data[byte_count], offset + byte_count, chunk_size);
        if (ret != 0) {
            Serial.println("Error writing memory mapped firmware data");
            break;
        }
        byte_count += chunk_size;
        printProgress(byte_count, file_size, 10, false);
    }
}

static void writeCertificates()
{
    FILE *fp = fopen("/wlan/cacert.pem", "wb");
    if (!fp) {
        Serial.println("Error opening /wlan/cacert.pem");
        return;
    }

    int chunk_size = 128;
    int byte_count = 0;

    Serial.println("Flashing certificates");
    printProgress(byte_count, cacert_pem_len, 10, true);
    while (byte_count < static_cast<int>(cacert_pem_len)) {
        if (byte_count + chunk_size > static_cast<int>(cacert_pem_len)) {
            chunk_size = cacert_pem_len - byte_count;
        }
        int ret = fwrite(&cacert_pem[byte_count], chunk_size, 1, fp);
        if (ret != 1) {
            Serial.println("Error writing certificates");
            break;
        }
        byte_count += chunk_size;
        printProgress(byte_count, cacert_pem_len, 10, false);
    }
    fclose(fp);
}

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("Installing Arduino GIGA WiFi firmware...");
    mbed::MBRBlockDevice::partition(&root, 1, 0x0B, 0, 1024 * 1024);

    int err = wifi_data_fs.mount(&wifi_data);
    if (err) {
        Serial.println("No filesystem containing the WiFi firmware was found.");
        Serial.println("Formatting the filesystem to install firmware and certificates.");
        err = wifi_data_fs.reformat(&wifi_data);
        if (err) {
            Serial.println("Error formatting WiFi firmware filesystem");
            return;
        }
    } else {
        Serial.println("Existing WiFi firmware filesystem mounted; refreshing contents.");
        wifi_data_fs.reformat(&wifi_data);
    }

    writeFirmwareFile();
    writeMemoryMappedFirmware();
    writeCertificates();

    Serial.println("Firmware and certificates updated.");
    Serial.println("Upload the main SPEExpertControl firmware again now.");
}

void loop()
{
}
