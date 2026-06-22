/*
 * Amplifier model catalog and identifier parsing helpers.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "models/amplifier_model.h"

#include <string.h>

#define COUNT_OF_LOCAL(a) (sizeof(a) / sizeof((a)[0]))

static const AmpModelInfo MODEL_CATALOG[] = {
    {AmpModelId::SpeExpert1k, "spe_expert_1k", "SPE Expert 1K-FA", true},
    {AmpModelId::SpeExpert13k, "spe_expert_13k", "SPE Expert 1.3K-FA", false},
    {AmpModelId::SpeExpert15k, "spe_expert_15k", "SPE Expert 1.5K-FA", false},
    {AmpModelId::SpeExpert2k, "spe_expert_2k", "SPE Expert 2K-FA", false},
};

const AmpModelInfo *amp_model_catalog(uint8_t &count)
{
    count = static_cast<uint8_t>(COUNT_OF_LOCAL(MODEL_CATALOG));
    return MODEL_CATALOG;
}

const AmpModelInfo *amp_model_info(AmpModelId id)
{
    for (size_t i = 0; i < COUNT_OF_LOCAL(MODEL_CATALOG); ++i) {
        if (MODEL_CATALOG[i].id == id) {
            return &MODEL_CATALOG[i];
        }
    }
    return nullptr;
}

const char *amp_model_key(AmpModelId id)
{
    const AmpModelInfo *info = amp_model_info(id);
    return info ? info->key : "unknown";
}

const char *amp_model_label(AmpModelId id)
{
    const AmpModelInfo *info = amp_model_info(id);
    return info ? info->label : "Unknown";
}

bool amp_model_available(AmpModelId id)
{
    const AmpModelInfo *info = amp_model_info(id);
    return info && info->available;
}

bool amp_model_parse(const char *value, AmpModelId &id)
{
    if (!value) {
        return false;
    }

    for (size_t i = 0; i < COUNT_OF_LOCAL(MODEL_CATALOG); ++i) {
        const AmpModelInfo &info = MODEL_CATALOG[i];
        if (strcmp(value, info.key) == 0 || strcmp(value, info.label) == 0) {
            id = info.id;
            return true;
        }
    }

    if (strcmp(value, "1k") == 0 || strcmp(value, "1k-fa") == 0 || strcmp(value, "spe1k") == 0) {
        id = AmpModelId::SpeExpert1k;
        return true;
    }
    if (strcmp(value, "13k") == 0 || strcmp(value, "1.3k") == 0 || strcmp(value, "1.3k-fa") == 0) {
        id = AmpModelId::SpeExpert13k;
        return true;
    }
    if (strcmp(value, "15k") == 0 || strcmp(value, "1.5k") == 0 || strcmp(value, "1.5k-fa") == 0) {
        id = AmpModelId::SpeExpert15k;
        return true;
    }
    if (strcmp(value, "2k") == 0 || strcmp(value, "2k-fa") == 0) {
        id = AmpModelId::SpeExpert2k;
        return true;
    }

    return false;
}

static bool packet_has_modern_id_at(const uint8_t *data, uint8_t len, uint8_t offset, const char *id)
{
    return data && len >= offset + 3 &&
           data[offset] == static_cast<uint8_t>(id[0]) &&
           data[offset + 1] == static_cast<uint8_t>(id[1]) &&
           data[offset + 2] == static_cast<uint8_t>(id[2]);
}

AmpModelId amp_model_detect_from_packet(const uint8_t *data, uint8_t len)
{
    if (!data) {
        return AmpModelId::Unknown;
    }

    if (len == 30) {
        return AmpModelId::SpeExpert1k;
    }

    if (len == 67) {
        const uint8_t id_offset = data[0] == ',' ? 1 : 0;
        if (packet_has_modern_id_at(data, len, id_offset, "13K")) {
            return AmpModelId::SpeExpert13k;
        }
        if (packet_has_modern_id_at(data, len, id_offset, "15K")) {
            return AmpModelId::SpeExpert15k;
        }
        if (packet_has_modern_id_at(data, len, id_offset, "20K")) {
            return AmpModelId::SpeExpert2k;
        }
    }

    return AmpModelId::Unknown;
}
