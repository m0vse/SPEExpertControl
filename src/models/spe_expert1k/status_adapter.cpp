/*
 * SPE Expert 1K-FA adapter from raw protocol packets to model-neutral amplifier status.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/spe_expert1k/status_adapter.h"

#include "app_status.h"
#include "models/spe_expert1k/status_view.h"
#include <stdio.h>
#include <string.h>

#define COUNT_OF_LOCAL(a) (sizeof(a) / sizeof((a)[0]))

static const char * const outs[] = {"HALF", "FULL"};
static const char * const tscales[] = {"\xC2\xB0" "F", "\xC2\xB0" "C"};
static const char * const inputs[] = {"1", "2"};
static const char * const antennas[] = {"1", "2", "3", "4"};
static const char * const bands[] = {"160 m", "80 m", "40 m", "30 m", "20 m", "17 m", "15 m", "12 m", "10 m", "6 m"};
static const char * const cats[] = {"SPE", "ICOM", "KENWD", "YAESU", "TTEC", "FLEX", "RS232", "NONE"};

AmpStatusSnapshot spe_expert1k_make_status_snapshot(bool valid, ExpertScreen screen, const Expert_Packet &packet)
{
    AmpStatusSnapshot snapshot;
    snapshot.valid = valid;
    snapshot.model = AmpModelId::SpeExpert1k;
    snapshot.screen_id = static_cast<uint8_t>(screen);
    snapshot.screen_name = screen_name(screen);

    if (!valid) {
        return snapshot;
    }

    const uint8_t band_idx = (packet.band_input >> 4) & 0x0f;
    const uint8_t input_idx = packet.band_input & 0x01;
    const uint8_t ants_idx = packet.antenna_cat & 0x07;
    const uint8_t cat_idx = (packet.antenna_cat >> 4) & 0x07;
    const uint8_t out_idx = (packet.flags >> 4) & 0x01;
    const bool swr_alarm = ((packet.flags >> 3) & 0x01) != 0;
    SpeStatusView status_view(packet);

    snapshot.flags = packet.flags;
    snapshot.display_context = packet.display_ctx;
    snapshot.op_status = status_view.op_status();
    snapshot.frequency = packet.freq;
    snapshot.sub_band = packet.sub_band;
    snapshot.input = input_idx < COUNT_OF_LOCAL(inputs) ? inputs[input_idx] : "?";
    snapshot.band = band_idx < COUNT_OF_LOCAL(bands) ? bands[band_idx] : "?";
    snapshot.antenna = ants_idx < COUNT_OF_LOCAL(antennas) ? antennas[ants_idx] : "?";
    snapshot.cat = cat_idx < COUNT_OF_LOCAL(cats) ? cats[cat_idx] : "?";
    snapshot.out = out_idx < COUNT_OF_LOCAL(outs) ? outs[out_idx] : "?";
    if (swr_alarm) {
        snprintf(snapshot.swr, sizeof(snapshot.swr), "--.--");
    } else {
        snprintf(snapshot.swr, sizeof(snapshot.swr), "%.2f", float(packet.swr_gain) / 100.0f);
    }
    snprintf(snapshot.temp, sizeof(snapshot.temp), "%d%s", packet.temp, tscales[(packet.flags >> 7) & 0x01]);
    snapshot.power = float(packet.power) / 10.0f;
    snapshot.reverse = float(packet.rev_power) / 10.0f;
    snapshot.voltage = float(packet.voltage) / 10.0f;
    snapshot.current = float(packet.current) / 10.0f;

    snapshot.power_meter.label = status_view.power_label();
    snapshot.power_meter.value = float(status_view.power_raw_tenths()) / 10.0f;
    snapshot.power_meter.max = status_view.power_bar_max();
    snapshot.power_meter.suffix = status_view.power_value_suffix();
    for (uint8_t i = 0; i < 5; ++i) {
        snapshot.power_meter.scale[i] = status_view.power_scale_label(i);
    }

    snapshot.pa_meter.label = status_view.pa_label();
    snapshot.pa_meter.value = float(status_view.pa_raw_tenths()) / 10.0f;
    snapshot.pa_meter.max = status_view.pa_bar_max();
    snapshot.pa_meter.suffix = status_view.pa_value_suffix();
    for (uint8_t i = 0; i < 5; ++i) {
        snapshot.pa_meter.scale[i] = status_view.pa_scale_label(i);
    }

    return snapshot;
}

static AppModelData spe_expert1k_make_model_data(ExpertScreen screen, const Expert_Packet &packet)
{
    AppModelData data;
    data.model = AmpModelId::SpeExpert1k;
    data.screen_id = static_cast<uint8_t>(screen);
    data.screen_name = screen_name(screen);
    data.size = sizeof(packet.setup) <= APP_MODEL_DATA_MAX ? sizeof(packet.setup) : APP_MODEL_DATA_MAX;
    memcpy(data.bytes, packet.setup, data.size);
    return data;
}

void spe_expert1k_publish_app_status(bool valid,
                                     ExpertScreen screen,
                                     const Expert_Packet &status,
                                     const Expert_Packet &web_cat_snapshot,
                                     unsigned long web_cat_snapshot_until)
{
    AppStatusSnapshot snapshot;
    snapshot.valid = valid;
    snapshot.amp = spe_expert1k_make_status_snapshot(valid, screen, status);
    snapshot.model_data = spe_expert1k_make_model_data(screen, status);
    snapshot.transient_valid = valid && web_cat_snapshot_until != 0;
    snapshot.transient_amp = spe_expert1k_make_status_snapshot(snapshot.transient_valid, Cat_Screen, web_cat_snapshot);
    snapshot.transient_model_data = spe_expert1k_make_model_data(Cat_Screen, web_cat_snapshot);
    snapshot.transient_until = web_cat_snapshot_until;
    app_status_publish(snapshot);
}
