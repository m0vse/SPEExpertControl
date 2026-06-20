# Repository Working Notes

- Do not run `pio clean` unless explicitly requested; LVGL rebuilds are slow on this machine.
- For the main firmware environment, default to uploading when verifying a build. Use:
  `pio run -e giga_r1_m7_spe_expert1k --target upload`
  unless the user explicitly asks for a build-only check.
- If the board is in DFU mode, upload the generated binary directly through the PlatformIO upload target rather than attempting serial reset diagnostics first.
