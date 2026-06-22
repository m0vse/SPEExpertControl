/*
 * Application status export API used to expose amplifier state as JSON.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "app_status.h"

#include <rtos.h>

#define COUNT_OF_LOCAL(a) (sizeof(a) / sizeof((a)[0]))

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
    const bool show_transient = snapshot.valid && snapshot.transient_valid && snapshot.transient_until != 0;
    AmpStatusSnapshot amp = show_transient ? snapshot.transient_amp : snapshot.amp;
    AppModelData model_data = show_transient ? snapshot.transient_model_data : snapshot.model_data;

    out.print(F("{\"valid\":"));
    out.print(snapshot.valid ? F("true") : F("false"));
    out.print(F(",\"screen\":"));
    out.print(amp.screen_id);
    out.print(F(",\"screenName\":"));
    json_print_string(out, amp.screen_name);
    out.print(F(",\"model\":"));
    json_print_string(out, amp_model_key(amp.model));
    out.print(F(",\"modelLabel\":"));
    json_print_string(out, amp_model_label(amp.model));

    if (!snapshot.valid) {
        out.print(F("}"));
        return;
    }

    out.print(F(",\"flags\":"));
    out.print(amp.flags);
    out.print(F(",\"displayCtx\":"));
    out.print(amp.display_context);
    out.print(F(",\"opStatus\":"));
    out.print(amp.op_status);
    out.print(F(",\"freq\":"));
    out.print(amp.frequency);
    out.print(F(",\"subBand\":"));
    out.print(amp.sub_band);
    out.print(F(",\"setup\":["));
    for (uint8_t i = 0; i < model_data.size; ++i) {
        if (i) {
            out.print(',');
        }
        out.print(model_data.bytes[i]);
    }
    out.print(']');
    out.print(F(",\"input\":"));
    json_print_string(out, amp.input);
    out.print(F(",\"band\":"));
    json_print_string(out, amp.band);
    out.print(F(",\"antenna\":"));
    json_print_string(out, amp.antenna);
    out.print(F(",\"cat\":"));
    json_print_string(out, amp.cat);
    out.print(F(",\"out\":"));
    json_print_string(out, amp.out);
    out.print(F(",\"power\":"));
    out.print(amp.power, 1);
    out.print(F(",\"reverse\":"));
    out.print(amp.reverse, 1);
    out.print(F(",\"swr\":"));
    json_print_string(out, amp.swr);
    out.print(F(",\"temp\":"));
    json_print_string(out, amp.temp);
    out.print(F(",\"voltage\":"));
    out.print(amp.voltage, 1);
    out.print(F(",\"current\":"));
    out.print(amp.current, 1);
    out.print(F(",\"meterPowerLabel\":"));
    json_print_string(out, amp.power_meter.label);
    out.print(F(",\"meterPower\":"));
    out.print(amp.power_meter.value, 1);
    out.print(F(",\"meterPowerMax\":"));
    out.print(amp.power_meter.max);
    out.print(F(",\"meterPowerSuffix\":"));
    json_print_string(out, amp.power_meter.suffix);
    out.print(F(",\"meterPowerScale\":["));
    for (uint8_t i = 0; i < 5; ++i) {
        if (i) {
            out.print(',');
        }
        json_print_string(out, amp.power_meter.scale[i]);
    }
    out.print(']');
    out.print(F(",\"meterPaLabel\":"));
    json_print_string(out, amp.pa_meter.label);
    out.print(F(",\"meterPa\":"));
    out.print(amp.pa_meter.value, 1);
    out.print(F(",\"meterPaMax\":"));
    out.print(amp.pa_meter.max);
    out.print(F(",\"meterPaSuffix\":"));
    json_print_string(out, amp.pa_meter.suffix);
    out.print(F(",\"meterPaScale\":["));
    for (uint8_t i = 0; i < 5; ++i) {
        if (i) {
            out.print(',');
        }
        json_print_string(out, amp.pa_meter.scale[i]);
    }
    out.print(']');
    out.print(F("}"));
}

uint32_t app_status_sequence(void)
{
    return app_status_snapshot().sequence;
}
