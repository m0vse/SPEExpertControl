/*
 * Application status export API used to expose amplifier state as JSON.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "app_status.h"

#include <rtos.h>
#include <string.h>

#define COUNT_OF_LOCAL(a) (sizeof(a) / sizeof((a)[0]))

static rtos::Mutex status_mutex;
static AppStatusSnapshot published;

static void copy_text(char *dst, size_t dst_len, const char *src)
{
    if (!dst || dst_len == 0) {
        return;
    }
    if (!src) {
        src = "";
    }
    strncpy(dst, src, dst_len - 1);
    dst[dst_len - 1] = '\0';
}

static void copy_amp_text(AppAmpTextStorage &storage, const AmpStatusSnapshot &amp)
{
    copy_text(storage.screen_name, sizeof(storage.screen_name), amp.screen_name);
    copy_text(storage.input, sizeof(storage.input), amp.input);
    copy_text(storage.band, sizeof(storage.band), amp.band);
    copy_text(storage.antenna, sizeof(storage.antenna), amp.antenna);
    copy_text(storage.cat, sizeof(storage.cat), amp.cat);
    copy_text(storage.out, sizeof(storage.out), amp.out);
    copy_text(storage.lcd_title, sizeof(storage.lcd_title), amp.lcd_title);
    copy_text(storage.lcd_body, sizeof(storage.lcd_body), amp.lcd_body);
    copy_text(storage.lcd_cells, sizeof(storage.lcd_cells), amp.lcd_cells);
    copy_text(storage.lcd_attrs, sizeof(storage.lcd_attrs), amp.lcd_attrs);
    copy_text(storage.lcd_footer, sizeof(storage.lcd_footer), amp.lcd_footer);
    copy_text(storage.lcd_hint, sizeof(storage.lcd_hint), amp.lcd_hint);
}

static void bind_amp_text(AmpStatusSnapshot &amp, AppAmpTextStorage &storage)
{
    amp.screen_name = storage.screen_name;
    amp.input = storage.input;
    amp.band = storage.band;
    amp.antenna = storage.antenna;
    amp.cat = storage.cat;
    amp.out = storage.out;
    amp.lcd_title = storage.lcd_title;
    amp.lcd_body = storage.lcd_body;
    amp.lcd_cells = storage.lcd_cells;
    amp.lcd_attrs = storage.lcd_attrs;
    amp.lcd_footer = storage.lcd_footer;
    amp.lcd_hint = storage.lcd_hint;
}

static void copy_model_text(char *dst, size_t dst_len, const AppModelData &model_data)
{
    copy_text(dst, dst_len, model_data.screen_name);
}

static void bind_snapshot_text(AppStatusSnapshot &snapshot)
{
    bind_amp_text(snapshot.amp, snapshot.amp_text);
    bind_amp_text(snapshot.transient_amp, snapshot.transient_amp_text);
    snapshot.model_data.screen_name = snapshot.model_screen_name;
    snapshot.transient_model_data.screen_name = snapshot.transient_model_screen_name;
}

static void json_print_string(Print &out, const char *value)
{
    out.print('"');
    if (value) {
        for (const char *p = value; *p; ++p) {
            if (*p == '"' || *p == '\\') {
                out.print('\\');
                out.print(*p);
            } else if (*p == '\n') {
                out.print(F("\\n"));
            } else if (*p == '\r') {
                out.print(F("\\r"));
            } else {
                out.print(*p);
            }
        }
    }
    out.print('"');
}

void app_status_publish(const AppStatusSnapshot &snapshot)
{
    status_mutex.lock();
    const uint32_t next_sequence = published.sequence + 1;
    published = snapshot;
    copy_amp_text(published.amp_text, snapshot.amp);
    copy_amp_text(published.transient_amp_text, snapshot.transient_amp);
    copy_model_text(published.model_screen_name, sizeof(published.model_screen_name), snapshot.model_data);
    copy_model_text(published.transient_model_screen_name,
                    sizeof(published.transient_model_screen_name),
                    snapshot.transient_model_data);
    bind_snapshot_text(published);
    published.sequence = next_sequence;
    status_mutex.unlock();
}

AppStatusSnapshot app_status_snapshot(void)
{
    status_mutex.lock();
    AppStatusSnapshot snapshot = published;
    status_mutex.unlock();
    bind_snapshot_text(snapshot);
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
    out.print(F(",\"lcdTitle\":"));
    json_print_string(out, amp.lcd_title);
    out.print(F(",\"lcdBody\":"));
    json_print_string(out, amp.lcd_body);
    out.print(F(",\"lcdCells\":"));
    json_print_string(out, amp.lcd_cells);
    out.print(F(",\"lcdAttrs\":"));
    json_print_string(out, amp.lcd_attrs);
    out.print(F(",\"lcdFooter\":"));
    json_print_string(out, amp.lcd_footer);
    out.print(F(",\"lcdHint\":"));
    json_print_string(out, amp.lcd_hint);
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
