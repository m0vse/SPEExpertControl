/*
 * Abstract runtime contract for amplifier model implementations.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <Arduino.h>

#include "models/amplifier_model.h"

class AmplifierRuntime {
public:
  virtual ~AmplifierRuntime() = default;

  virtual AmpModelId model_id() const = 0;
  virtual bool status_valid() const = 0;
  virtual uint8_t screen_id() const = 0;
  virtual const char *screen_name() const = 0;
  virtual unsigned long last_activity_ms() const = 0;
  virtual void publish_status() const = 0;
};
