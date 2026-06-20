# SPEExpertControl

Copyright (C) 2026 Phil Taylor (M0VSE)

SPEExpertControl is a PlatformIO firmware project for controlling SPE Expert amplifiers from an Arduino Giga R1 with the Arduino Giga Display Shield. The current supported amplifier model is the SPE Expert 1K-FA.

The project provides a local LVGL touch interface, serial control of the amplifier, WiFi setup, a serial debug console, and a lightweight browser UI that mirrors the LCD screens.

![main-screen](https://github.com/user-attachments/assets/f7b14f2b-df57-4a4f-8e39-0634fccd2e02)

## Hardware

- Arduino Giga R1, M7 core
- Arduino Giga Display Shield
- Two-channel MAX3232 level shifter connected to the first UART and amplifier DTR
- SPE Expert 1K-FA amplifier

### Amplifier RS-232 Wiring

The amplifier serial port uses RS-232 voltage levels. Use a two-channel MAX3232 board so the normal UART and the DTR power-on signal are both level shifted.

Recommended Arduino Giga wiring:

| Arduino Giga | MAX3232 side | Amplifier D-type | Purpose |
| --- | --- | --- | --- |
| `3V3` | `VCC` | - | MAX3232 module power |
| `GND` | `GND` | DB-9 pin 5 | Common signal ground |
| `Serial1 TX` | `T1IN` | Amplifier RXD | Command/status UART transmit |
| `Serial1 RX` | `R1OUT` | Amplifier TXD | Command/status UART receive |
| `D7` | `T2IN` | Amplifier DTR | Power-on / remote wake request |

The MAX3232 outputs should connect to the amplifier side of the D-type socket:

| MAX3232 RS-232 side | Amplifier DB-9 pin | Purpose |
| --- | --- | --- |
| `T1OUT` | Pin 3, RXD | UART data from controller to amplifier |
| `R1IN` | Pin 2, TXD | UART data from amplifier to controller |
| `T2OUT` | Pin 4, DTR | DTR asserted by controller |
| `GND` | Pin 5, GND | Common signal ground |

`D7` is assigned as `SPE_AMP_DTR_PIN`. The firmware asserts DTR during controller startup to request remote power-on. When the OFF button command is sent, the firmware sends `RCU_OFF`, releases DTR, and stops the periodic `RCU_ON` polling so the amplifier can power down. Pressing ON asserts DTR again and restarts remote console updates.

Because the MAX3232 transmitter inverts the logic level, the firmware drives `D7` low to assert RS-232 DTR. If the wiring is moved later, change `SPE_AMP_DTR_PIN` in [src/main.cpp](src/main.cpp).

## Current Features

- LVGL 9.5 UI on the Giga Display Shield
- SPE Expert 1K-FA serial packet parsing and button command transmission
- Touch control for amplifier buttons and setup menus
- FreeRTOS split between UI and amplifier serial work
- Mutex handling around LVGL, amplifier serial, and debug serial access
- Saved WiFi credentials stored on the Giga QSPI filesystem
- Hidden WiFi setup popup opened from the top-left of the display
- Background WiFi reconnect using saved credentials
- Serial console commands for WiFi, amplifier status, scans, and diagnostics
- DTR power-on output for waking the amplifier when it is switched off
- Web UI that mirrors the LCD screens and sends the same amplifier button commands
- Separate PlatformIO environment for refreshing the Arduino Giga WiFi firmware

## Project Layout

- `src/main.cpp` - application entry point, RTOS tasks, amplifier packet handling, LCD screen updates, and serial console
- `src/display/` - LVGL display and touch adapters for the Arduino Giga Display Shield
- `src/network/` - lightweight HTTP server for browser control
- `src/ui/` - LVGL screen construction, UI helpers, WiFi setup popup, and UI assets
- `src/models/spe_expert1k/` - SPE Expert 1K-FA model-specific menu metadata
- `include/` - public headers, LVGL declarations, model definitions, and configuration
- `tools/` - PlatformIO helper scripts used during builds
- `platformio.ini` - PlatformIO environments and build configuration

## Build

PlatformIO is the supported build path.

```powershell
pio run -e giga_r1_m7_spe_expert1k
```

The default environment is `giga_r1_m7_spe_expert1k`. It targets the Arduino Giga R1 M7 core and currently uses `COM27` for upload and monitor.

## Upload

```powershell
pio run -e giga_r1_m7_spe_expert1k --target upload
```

The environment uses DFU upload. If the board is already in DFU mode, PlatformIO should still upload the generated firmware binary.

## Serial Console

The debug serial monitor runs at `115200` baud.

Useful commands:

- `help` - show available console commands
- `status` - print controller summary
- `wifi` - print WiFi status, firmware version, IP, and RSSI
- `amp` - print the last amplifier status packet
- `scan` - run a blocking WiFi scan and print results
- `rcu` - send `RCU_ON` to the amplifier
- `dtr` - print the configured amplifier DTR output pin and GPIO level
- `wifi-popup` - open the hidden WiFi setup popup
- `wifi-saved` - show whether saved WiFi credentials exist
- `wifi-clear` - clear saved WiFi credentials
- `poll on` / `poll off` - enable or disable periodic console status polling

## WiFi

WiFi is configured from a hidden popup on the LCD. Tap the top-left of the display to open it. Use `Search` to scan, select an SSID, enter the password, and press `Connect`.

Credentials are stored on the Giga QSPI filesystem. On restart, the firmware loads saved credentials and attempts to reconnect in the background. Failed attempts time out and are retried periodically without blocking the amplifier UI or serial control.

## Web UI

When WiFi is connected, the firmware starts a lightweight HTTP server on port 80. The device prints the URL to the serial console after connection.

The web UI mirrors the LCD screen state and sends commands through the same amplifier control path as the physical/touch buttons. Status is exposed as JSON through `/api/status`, and button commands are sent through `/api/key?name=...`.

## WiFi Firmware Updater

If the board reports that the filesystem containing the WiFi firmware cannot be mounted, build and upload the updater environment:

```powershell
pio run -e giga_r1_m7_wifi_firmware_updater --target upload
```

After the updater finishes, upload the main firmware again:

```powershell
pio run -e giga_r1_m7_spe_expert1k --target upload
```

## Build Notes

The firmware uses the official Arduino display and touch libraries from PlatformIO dependencies rather than local library copies. A PlatformIO pre-build script patches the installed `Arduino_H7_Video` package so LVGL display initialisation can be controlled by this firmware.

Flash usage is relatively high because LVGL, WiFi, the web UI, and the local LCD assets are all included. The build enables section garbage collection and size optimisation.

## License

This project is licensed under GPL-3.0-only. See [LICENSE](LICENSE).
