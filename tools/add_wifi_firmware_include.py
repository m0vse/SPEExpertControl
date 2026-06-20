# PlatformIO pre-build helper that adds the Arduino WiFi firmware updater example include path.
#
# Copyright (C) 2026 Phil Taylor (M0VSE)
# SPDX-License-Identifier: GPL-3.0-only

from pathlib import Path

Import("env")

framework_dir = Path(env.PioPlatform().get_package_dir("framework-arduino-mbed"))
env.Append(CPPPATH=[
    str(framework_dir / "libraries" / "STM32H747_System" / "examples" / "WiFiFirmwareUpdater")
])
