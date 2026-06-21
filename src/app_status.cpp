/*
 * Application status export API used to expose amplifier state as JSON.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "app_status.h"

#include "models/spe_expert1k/status_view.h"
#include <rtos.h>
#include <stdio.h>

#define COUNT_OF_LOCAL(a) (sizeof(a) / sizeof((a)[0]))

static const char * const outs[] = {"HALF", "FULL"};
static const char * const tscales[] = {"°F", "°C"};
static const char * const inputs[] = {"1", "2"};
static const char * const antennas[] = {"1", "2", "3", "4"};
static const char * const bands[] = {"160 m", "80 m", "40 m", "30 m", "20 m", "17 m", "15 m", "12 m", "10 m", "6 m"};
static const char * const cats[] = {"SPE", "ICOM", "KENWD", "YAESU", "TTEC", "FLEX", "RS232", "NONE"};

static rtos::Mutex status_mutex;
static AppStatusSnapshot published;

static void json_print_string(Print &out, const char *value)
{
    out.print('"');
    if (value) {
        for (const char *p = value; *p; ++p) {
            if (*p == '"' || *p == '\\') {
                out.print('\\');
            }
            out.print(*p);
        }
    }
    out.print('"');
}

void app_status_publish(const AppStatusSnapshot &snapshot)
{
    status_mutex.lock();
    published = snapshot;
    ++published.sequence;
    status_mutex.unlock();
}

AppStatusSnapshot app_status_snapshot(void)
{
    status_mutex.lock();
    AppStatusSnapshot snapshot = published;
    status_mutex.unlock();
    return snapshot;
}

void app_status_print_json(Print &out)
{
    AppStatusSnapshot snapshot = app_status_snapshot();
    ExpertScreen current_screen = snapshot.screen;
    const bool show_cat_snapshot = snapshot.valid && current_screen == Cat_Screen && snapshot.web_cat_snapshot_until != 0;
    Expert_Packet status = show_cat_snapshot ? snapshot.web_cat_snapshot : snapshot.status;
    if (show_cat_snapshot) {
        current_screen = Cat_Screen;
    }

    out.print(F("{\"valid\":"));
    out.print(snapshot.valid ? F("true") : F("false"));
    out.print(F(",\"screen\":"));
    out.print(static_cast<int>(current_screen));
    out.print(F(",\"screenName\":"));
    json_print_string(out, screen_name(current_screen));

    if (!snapshot.valid) {
        out.print(F("}"));
        return;
    }

    const uint8_t band_idx = (status.band_input >> 4) & 0x0f;
    const uint8_t input_idx = status.band_input & 0x01;
    const uint8_t ants_idx = status.antenna_cat & 0x07;
    const uint8_t cat_idx = (status.antenna_cat >> 4) & 0x07;
    const uint8_t out_idx = (status.flags >> 4) & 0x01;
    const bool swr_alarm = ((status.flags >> 3) & 0x01) != 0;
    SpeStatusView status_view(status);

    out.print(F(",\"flags\":"));
    out.print(status.flags);
    out.print(F(",\"displayCtx\":"));
    out.print(status.display_ctx);
    out.print(F(",\"opStatus\":"));
    out.print(status_view.op_status());
    out.print(F(",\"freq\":"));
    out.print(status.freq);
    out.print(F(",\"subBand\":"));
    out.print(status.sub_band);
    out.print(F(",\"setup\":["));
    for (uint8_t i = 0; i < COUNT_OF_LOCAL(status.setup); ++i) {
        if (i) {
            out.print(',');
        }
        out.print(status.setup[i]);
    }
    out.print(']');
    out.print(F(",\"input\":"));
    json_print_string(out, input_idx < COUNT_OF_LOCAL(inputs) ? inputs[input_idx] : "?");
    out.print(F(",\"band\":"));
    json_print_string(out, band_idx < COUNT_OF_LOCAL(bands) ? bands[band_idx] : "?");
    out.print(F(",\"antenna\":"));
    json_print_string(out, ants_idx < COUNT_OF_LOCAL(antennas) ? antennas[ants_idx] : "?");
    out.print(F(",\"cat\":"));
    json_print_string(out, cat_idx < COUNT_OF_LOCAL(cats) ? cats[cat_idx] : "?");
    out.print(F(",\"out\":"));
    json_print_string(out, out_idx < COUNT_OF_LOCAL(outs) ? outs[out_idx] : "?");
    out.print(F(",\"power\":"));
    out.print(float(status.power) / 10.0f, 1);
    out.print(F(",\"reverse\":"));
    out.print(float(status.rev_power) / 10.0f, 1);
    out.print(F(",\"swr\":"));
    if (swr_alarm) {
        json_print_string(out, "--.--");
    } else {
        char swr[8];
        snprintf(swr, sizeof(swr), "%.2f", float(status.swr_gain) / 100.0f);
        json_print_string(out, swr);
    }
    out.print(F(",\"temp\":"));
    char temp[12];
    snprintf(temp, sizeof(temp), "%d%s", status.temp, tscales[(status.flags >> 7) & 0x01]);
    json_print_string(out, temp);
    out.print(F(",\"voltage\":"));
    out.print(float(status.voltage) / 10.0f, 1);
    out.print(F(",\"current\":"));
    out.print(float(status.current) / 10.0f, 1);
    out.print(F(",\"meterPowerLabel\":"));
    json_print_string(out, status_view.power_label());
    out.print(F(",\"meterPower\":"));
    out.print(float(status_view.power_raw_tenths()) / 10.0f, 1);
    out.print(F(",\"meterPowerMax\":"));
    out.print(status_view.power_bar_max());
    out.print(F(",\"meterPowerSuffix\":"));
    json_print_string(out, status_view.power_value_suffix());
    out.print(F(",\"meterPowerScale\":["));
    for (uint8_t i = 0; i < 5; ++i) {
        if (i) {
            out.print(',');
        }
        json_print_string(out, status_view.power_scale_label(i));
    }
    out.print(']');
    out.print(F(",\"meterPaLabel\":"));
    json_print_string(out, status_view.pa_label());
    out.print(F(",\"meterPa\":"));
    out.print(float(status_view.pa_raw_tenths()) / 10.0f, 1);
    out.print(F(",\"meterPaMax\":"));
    out.print(status_view.pa_bar_max());
    out.print(F(",\"meterPaSuffix\":"));
    json_print_string(out, status_view.pa_value_suffix());
    out.print(F(",\"meterPaScale\":["));
    for (uint8_t i = 0; i < 5; ++i) {
        if (i) {
            out.print(',');
        }
        json_print_string(out, status_view.pa_scale_label(i));
    }
    out.print(']');
    out.print(F("}"));
}

uint32_t app_status_sequence(void)
{
    return app_status_snapshot().sequence;
}
