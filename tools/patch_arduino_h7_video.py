# PlatformIO pre-build patch for Arduino_H7_Video so the Giga display stack can be initialised manually for LVGL 9.
#
# Copyright (C) 2026 Phil Taylor (M0VSE)
# SPDX-License-Identifier: GPL-3.0-only

import re
from pathlib import Path

Import("env")


framework_dir = Path(env.PioPlatform().get_package_dir("framework-arduino-mbed"))
video_cpp = framework_dir / "libraries" / "Arduino_H7_Video" / "src" / "Arduino_H7_Video.cpp"

source = video_cpp.read_text()

late_non_giga_sdram = """#if !defined(ARDUINO_GIGA)
  /* Configure SDRAM */
  SDRAM.begin(dsi_getFramebufferEnd()); //FIXME: SDRAM init after video controller init can cause display glitch at start-up
#endif
"""

sdram_after_lvgl = """  /* Configure SDRAM after DSI has set the real framebuffer size. */
  SDRAM.begin(dsi_getFramebufferEnd()); //FIXME: SDRAM init after video controller init can cause display glitch at start-up
"""

sdram_before_lvgl = """  /* Configure SDRAM after DSI has set the real framebuffer size, before LVGL allocations. */
  SDRAM.begin(dsi_getFramebufferEnd()); //FIXME: SDRAM init after video controller init can cause display glitch at start-up

"""

source = re.sub(
    r"#if defined\(ARDUINO_GIGA\)[ \t]*\n"
    r"  /\* Configure SDRAM \*/\n"
    r"  SDRAM\.begin\(dsi_getFramebufferEnd\(\)\);\n"
    r"#endif\n\n",
    "",
    source,
)

if late_non_giga_sdram in source:
    source = source.replace(late_non_giga_sdram, "")

if sdram_after_lvgl in source:
    source = source.replace(sdram_after_lvgl, "")

init_done = """  int err_code = _shield->init(_edidMode);
  if (err_code < 0) {
    return 3; /* Video controller fail init */
  }

"""

if sdram_before_lvgl not in source and init_done in source:
    source = source.replace(init_done, init_done + sdram_before_lvgl, 1)

auto_lvgl_begin = """  #if __has_include("lvgl.h")
    /* Initiliaze LVGL library */
"""

manual_lvgl_begin = """  #if __has_include("lvgl.h") && !defined(SPE_MANUAL_LVGL_DISPLAY)
    /* Initiliaze LVGL library */
"""

if auto_lvgl_begin in source:
    source = source.replace(auto_lvgl_begin, manual_lvgl_begin, 1)

force_rotation = """        rotation = LV_DISPLAY_ROTATION_90; // Giga Display Shield framebuffer orientation
"""

source = re.sub(
    r"        #if \(LVGL_VERSION_MINOR < 2\)[ \t]*\n"
    r"        rotation = LV_DISPLAY_ROTATION_90; // bugfix: force 90 degree rotation for lvgl 9\.1 end earlier\n"
    r"        #endif\n",
    force_rotation,
    source,
)

video_cpp.write_text(source)
