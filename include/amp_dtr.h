/*
 * Amplifier DTR GPIO control used for SPE remote power-on signalling.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <Arduino.h>

#ifndef SPE_AMP_DTR_PIN
#define SPE_AMP_DTR_PIN 7
#endif

#ifndef SPE_AMP_DTR_ASSERTED_LEVEL
#define SPE_AMP_DTR_ASSERTED_LEVEL LOW
#endif

void amp_dtr_begin();
void amp_dtr_set(bool asserted);
bool amp_dtr_is_asserted();
uint8_t amp_dtr_pin();
int amp_dtr_gpio_level();
