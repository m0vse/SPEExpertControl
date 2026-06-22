/*
 * Lightweight HTTP server that mirrors the LCD UI and exposes amplifier control endpoints.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "network/control_server.h"

#include "amp_control.h"
#include "app_config.h"
#include "app_status.h"
#include "network/control_page.h"
#include "network/spe_logo_svg.h"
#include "network/wifi_lock.h"
#include <Arduino.h>
#include <WiFi.h>
#include <rtos.h>

struct ControlServerStats {
    uint32_t server_starts = 0;
    uint32_t wifi_disconnects = 0;
    uint32_t clients = 0;
    uint32_t empty_requests = 0;
    uint32_t index_requests = 0;
    uint32_t status_requests = 0;
    uint32_t key_requests = 0;
    uint32_t logo_requests = 0;
    uint32_t bad_key_requests = 0;
    uint32_t last_request_ms = 0;
    uint16_t active_port = APP_CONFIG_DEFAULT_WEB_PORT;
    char last_request[32] = "";
};

static rtos::Mutex stats_mutex;
static ControlServerStats stats;

static bool debug_background_logs_enabled()
{
    return app_config_amp_serial_port() != AppAmpSerialPort::Usb;
}

static void updateStats(void (*update)(ControlServerStats &))
{
    stats_mutex.lock();
    update(stats);
    stats_mutex.unlock();
}

static void recordRequest(const char *path)
{
    stats_mutex.lock();
    ++stats.clients;
    stats.last_request_ms = millis();
    strncpy(stats.last_request, path ? path : "", sizeof(stats.last_request) - 1);
    stats.last_request[sizeof(stats.last_request) - 1] = '\0';
    stats_mutex.unlock();
}

class StringPrint : public Print {
public:
    explicit StringPrint(String &out) : out_(out) {}

    size_t write(uint8_t value) override
    {
        out_ += static_cast<char>(value);
        return 1;
    }

    size_t write(const uint8_t *buffer, size_t size) override
    {
        if (!buffer) {
            return 0;
        }
        out_.concat(reinterpret_cast<const char *>(buffer), size);
        return size;
    }

private:
    String &out_;
};

static void appendJsonString(String &out, const String &value)
{
    out += '"';
    for (size_t i = 0; i < value.length(); ++i) {
        const char c = value[i];
        if (c == '"' || c == '\\') {
            out += '\\';
        }
        out += c;
    }
    out += '"';
}

class ControlServer {
public:
    void service()
    {
        const int wifi_status = WiFi.status();
        if (wifi_status != WL_CONNECTED) {
            if (last_wifi_status_ == WL_CONNECTED) {
                updateStats([](ControlServerStats &s) { ++s.wifi_disconnects; });
            }
            last_wifi_status_ = wifi_status;
            started_ = false;
            return;
        }

        if (last_wifi_status_ != WL_CONNECTED || !started_) {
            printWifiStatus();
            last_wifi_status_ = wifi_status;
            begin();
        }

        if (!server_) {
            return;
        }

        WiFiClient client = server_->accept();
        if (!client) {
            return;
        }

        handleClient(client);
    }

private:
    void begin()
    {
        if (started_) {
            return;
        }

        active_port_ = app_config_web_port();
        if (server_) {
            delete server_;
        }
        server_ = new WiFiServer(active_port_);
        server_->begin();
        started_ = true;
        updateStats([](ControlServerStats &s) {
            ++s.server_starts;
            s.active_port = app_config_web_port();
        });
        if (debug_background_logs_enabled()) {
            Serial.println(F("HTTP control server started"));
        }
    }

    void printWifiStatus()
    {
        if (!debug_background_logs_enabled()) {
            return;
        }
        IPAddress ip = WiFi.localIP();
        Serial.print(F("WiFi connected: "));
        Serial.println(WiFi.SSID());
        Serial.print(F("IP address: "));
        Serial.println(ip);
        Serial.print(F("RSSI: "));
        Serial.print(WiFi.RSSI());
        Serial.println(F(" dBm"));
        Serial.print(F("Web UI: http://"));
        Serial.print(ip);
        if (active_port_ != 80) {
            Serial.print(':');
            Serial.print(active_port_);
        }
        Serial.println();
    }

    void handleClient(WiFiClient &client)
    {
        String request_line;
        const unsigned long started = millis();

        while (client.connected() && millis() - started < 120) {
            if (!client.available()) {
                delay(1);
                continue;
            }

            char c = client.read();
            if (c == '\r') {
                continue;
            }
            if (c == '\n') {
                break;
            }
            request_line += c;
        }

        while (client.available()) {
            client.read();
        }

        if (request_line.length() == 0) {
            updateStats([](ControlServerStats &s) {
                ++s.clients;
                ++s.empty_requests;
                s.last_request_ms = millis();
                strncpy(s.last_request, "(empty)", sizeof(s.last_request) - 1);
                s.last_request[sizeof(s.last_request) - 1] = '\0';
            });
            client.stop();
            return;
        }

        if (request_line.startsWith("GET /spe-logo.svg")) {
            recordRequest("/spe-logo.svg");
            updateStats([](ControlServerStats &s) { ++s.logo_requests; });
            sendLogo(client);
        } else if (request_line.startsWith("GET /api/status") || request_line.startsWith("GET /status.json")) {
            recordRequest("/api/status");
            updateStats([](ControlServerStats &s) { ++s.status_requests; });
            sendStatusJson(client);
        } else if (request_line.startsWith("GET /api/key?name=")) {
            recordRequest("/api/key");
            updateStats([](ControlServerStats &s) { ++s.key_requests; });
            handleKeyRequest(client, request_line);
        } else {
            recordRequest("/");
            updateStats([](ControlServerStats &s) { ++s.index_requests; });
            sendIndex(client);
        }

        client.stop();
    }

    void sendStatusJson(WiFiClient &client)
    {
        String body;
        body.reserve(2300);
        appendStatusJson(body, false, "");

        client.println(F("HTTP/1.1 200 OK"));
        client.println(F("Content-Type: application/json"));
        client.print(F("Content-Length: "));
        client.println(body.length());
        client.println(F("Connection: close"));
        client.println();
        client.print(body);
    }

    void sendLogo(WiFiClient &client)
    {
        client.println(F("HTTP/1.1 200 OK"));
        client.println(F("Content-Type: image/svg+xml"));
        client.print(F("Content-Length: "));
        client.println(SPE_LOGO_SVG_SIZE);
        client.println(F("Cache-Control: max-age=86400"));
        client.println(F("Connection: close"));
        client.println();
        client.print(SPE_LOGO_SVG);
    }

    void handleKeyRequest(WiFiClient &client, const String &request_line)
    {
        String name = request_line.substring(strlen("GET /api/key?name="));
        const int end = name.indexOf(' ');
        if (end >= 0) {
            name.remove(end);
        }

        const uint32_t start_sequence = app_status_sequence();
        const bool ok = amp_control_press_key(name.c_str());
        if (!ok) {
            updateStats([](ControlServerStats &s) { ++s.bad_key_requests; });
        }
        (void)start_sequence;
        String body;
        body.reserve(2350);
        appendStatusJson(body, true, name, ok);

        client.println(ok ? F("HTTP/1.1 200 OK") : F("HTTP/1.1 400 Bad Request"));
        client.println(F("Content-Type: application/json"));
        client.print(F("Content-Length: "));
        client.println(body.length());
        client.println(F("Connection: close"));
        client.println();
        client.print(body);
    }

    void sendIndex(WiFiClient &client)
    {
        control_page_send(client);
    }

    void appendStatusJson(String &body, bool include_key, const String &key, bool ok = true)
    {
        if (include_key) {
            body += F("{\"ok\":");
            body += ok ? F("true") : F("false");
            body += F(",\"key\":");
            appendJsonString(body, key);
            body += ',';
        } else {
            body += '{';
        }

        body += F("\"wifi\":{\"status\":\"connected\",\"ssid\":");
        appendJsonString(body, WiFi.SSID());
        body += F(",\"ip\":");
        appendJsonString(body, WiFi.localIP().toString());
        body += F(",\"rssi\":");
        body += WiFi.RSSI();
        body += F("},\"amp\":");
        StringPrint body_print(body);
        app_status_print_json(body_print);
        body += '}';
    }

    WiFiServer *server_ = nullptr;
    uint16_t active_port_ = APP_CONFIG_DEFAULT_WEB_PORT;
    int last_wifi_status_ = WL_IDLE_STATUS;
    bool started_ = false;
};

static ControlServer control_server;

void control_server_service(void)
{
    WifiStackLock lock;
    control_server.service();
}

void control_server_print_stats(Print &out)
{
    ControlServerStats snapshot;
    stats_mutex.lock();
    snapshot = stats;
    stats_mutex.unlock();

    out.println(F("HTTP server stats:"));
    out.print(F("  server_starts="));
    out.println(snapshot.server_starts);
    out.print(F("  active_port="));
    out.println(snapshot.active_port);
    out.print(F("  wifi_disconnects="));
    out.println(snapshot.wifi_disconnects);
    out.print(F("  clients="));
    out.println(snapshot.clients);
    out.print(F("  empty_requests="));
    out.println(snapshot.empty_requests);
    out.print(F("  index="));
    out.print(snapshot.index_requests);
    out.print(F(" status="));
    out.print(snapshot.status_requests);
    out.print(F(" key="));
    out.print(snapshot.key_requests);
    out.print(F(" logo="));
    out.println(snapshot.logo_requests);
    out.print(F("  bad_key_requests="));
    out.println(snapshot.bad_key_requests);
    out.print(F("  last_request_ms="));
    out.print(snapshot.last_request_ms);
    out.print(F(" path="));
    out.println(snapshot.last_request);
}
