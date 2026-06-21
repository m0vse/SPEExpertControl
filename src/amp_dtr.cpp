/*
 * Amplifier DTR GPIO control used for SPE remote power-on signalling.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "amp_dtr.h"

static bool dtr_asserted = false;

void amp_dtr_begin()
{
  pinMode(SPE_AMP_DTR_PIN, OUTPUT);
  amp_dtr_set(true);
}

void amp_dtr_set(bool asserted)
{
  if (asserted) {
    digitalWrite(SPE_AMP_DTR_PIN, SPE_AMP_DTR_ASSERTED_LEVEL);
  } else {
    digitalWrite(SPE_AMP_DTR_PIN, SPE_AMP_DTR_ASSERTED_LEVEL == HIGH ? LOW : HIGH);
  }
  dtr_asserted = asserted;
}

bool amp_dtr_is_asserted()
{
  return dtr_asserted;
}

uint8_t amp_dtr_pin()
{
  return SPE_AMP_DTR_PIN;
}

int amp_dtr_gpio_level()
{
  return digitalRead(SPE_AMP_DTR_PIN);
}
