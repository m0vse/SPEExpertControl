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

### Enclosure

The tested enclosure is the KKSB Cases KKSB Case with Adjustable Stand for Arduino GIGA WiFi and Arduino GIGA Display Shield, listed on Amazon UK as ASIN [`B0CTMX2Z46`](https://www.amazon.co.uk/dp/B0CTMX2Z46/). It fits the controller well; the only required mechanical change is a hole in the perspex back panel for the amplifier DB-9 connector.

<img src="https://m.media-amazon.com/images/I/61t7FyWMJ3L._AC_SL1500_.jpg" alt="tested enclosure" width="360">

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

`D7` is assigned as `SPE_AMP_DTR_PIN`. The firmware asserts DTR during controller startup to request remote power-on. When the OFF button command is sent, the firmware releases DTR immediately, sends the OFF key command and `RCU_OFF`, and stops the periodic `RCU_ON` polling so the amplifier can power down. Pressing ON asserts DTR again and restarts remote console updates. This behaviour is deliberately driven by the button commands rather than the last received amplifier state, so OFF can still release DTR if the amplifier is not currently responding.

Because the MAX3232 transmitter inverts the logic level, the firmware drives `D7` low to assert RS-232 DTR. If the wiring is moved later, change `SPE_AMP_DTR_PIN` in [include/amp_dtr.h](include/amp_dtr.h).

## Current Features

- LVGL 9.5 UI on the Giga Display Shield
- SPE Expert 1K-FA serial packet parsing and button command transmission
- Model catalog with SPE Expert 1K-FA active and 1.3K-FA, 1.5K-FA, and 2K-FA reserved for future protocol handlers
- Touch control for amplifier buttons and setup menus
- FreeRTOS split between UI and amplifier serial work
- Mutex handling around LVGL, amplifier serial, and debug serial access
- Saved WiFi credentials stored on the Giga QSPI filesystem
- Hidden setup popup opened from the top-left of the display for WiFi and display orientation
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
pio run -e giga_r1_m7_spe_expert1k --target upload
```

The default environment is `giga_r1_m7_spe_expert1k`. It targets the Arduino Giga R1 M7 core and currently uses `COM27` for upload and monitor. During normal development, use the upload target above so a successful build is immediately deployed. Avoid `pio clean` unless a clean rebuild is specifically needed because LVGL rebuilds are slow.

## Build Only

Use a build-only command only when explicitly checking compilation without deploying:

```powershell
pio run -e giga_r1_m7_spe_expert1k
```

The environment uses DFU upload. If the board is already in DFU mode, PlatformIO should still upload the generated firmware binary.

## Amplifier Serial Transport

The normal amplifier link uses `Serial1` through the MAX3232 RS-232 interface. This is the default and is the recommended configuration for the SPE Expert 1K-FA hardware.

For model-protocol development or bench testing, the amplifier protocol can instead use `Serial2`, `Serial3`, `Serial4`, or the USB CDC `Serial` port. Open the controller setup popup and select the port from the `Amp serial` dropdown, or use the serial console command `amp-serial uart1`, `uart2`, `uart3`, `uart4`, or `usb`. Changing this setting saves it to QSPI storage and immediately reboots so the serial tasks restart on the selected transport.

The amplifier serial baud rate is also configurable. The default is `9600`, matching the current 1K-FA setup. Supported rates are `1200`, `2400`, `4800`, `9600`, `19200`, `38400`, `57600`, and `115200`. Change this from the setup popup baud dropdown or with `amp-baud <rate>`; changing it also saves and reboots.

In USB amplifier mode the normal debug console is disabled so it does not consume amplifier bytes. Press `Esc` three times on the USB serial terminal to open the console. Type `exit` or `passthrough` to return the USB port to amplifier communications. Do not use USB amplifier mode for normal operation while the amplifier is still wired to `Serial1`.

## Serial Console

The debug serial monitor runs on `Serial` at `115200` baud. PlatformIO is configured to monitor `COM27`:

```powershell
pio device monitor -e giga_r1_m7_spe_expert1k
```

Commands are line based, case-insensitive, and are submitted with Enter. Backspace is supported while entering a command.

When USB amplifier serial is enabled, press `Esc` three times to open the console before entering commands. The `exit` command releases the USB port back to the amplifier protocol.

### Console Commands

| Command | Alias | Description |
| --- | --- | --- |
| `help` | `?` | Show the firmware command list. |
| `status` | - | Print controller uptime, boot progress, touch state, transformed input-device count, current screen, and `serial1_available` when high bring-up diagnostics are enabled. |
| `wifi` | - | Print WiFi status code/name, WiFi firmware version, setup connection state, saved-credential state, and SSID/IP/RSSI when connected. |
| `web` | - | Print HTTP server counters, including starts, disconnects, client count, request counts, bad key requests, and the last request path/time. |
| `serial` | `ser` | Print amplifier UART health counters. Use this when checking checksum errors, queue depth, missed packets, or whether the serial task is falling behind. |
| `amp` | - | Print the last decoded 30-byte amplifier status packet, screen name, DTR state, band, input, antenna, CAT mode, output power setting, power, reflected power, SWR/gain, temperature, PA voltage, and PA current. |
| `model` | `amp-model` | Print active/saved amplifier model and known model IDs. `model spe_expert_1k` is currently the only selectable runtime; unsupported models are listed but rejected until their protocol runtime exists. |
| `scan` | - | Run a blocking WiFi scan and print SSID, RSSI, and encryption type for each network. The LCD and web UI continue to use the background WiFi services; this command is mainly for diagnostics. |
| `stats` | `mem` | Print Mbed runtime statistics: uptime, idle/sleep times, heap usage, stack usage, and RTOS thread state/priority/free stack. |
| `reboot` | `reset` | Reboot the controller with `NVIC_SystemReset()`. |
| `dfu` | `bootloader` | Reboot into the Giga DFU bootloader for firmware upload. |
| `rcu` | - | Queue an `RCU_ON` command to the amplifier. |
| `dtr` | - | Print the configured DTR pin, logical asserted state, and actual GPIO level. With the current MAX3232 wiring, asserted DTR is `D7 LOW`. |
| `amp-serial` | `ampserial` | Print the active and saved amplifier serial transport. Use `amp-serial uart1`, `uart2`, `uart3`, `uart4`, or `usb` to save a new transport and reboot. |
| `amp-baud` | `ampbaud` | Print the saved amplifier baud rate. Use `amp-baud 1200`, `2400`, `4800`, `9600`, `19200`, `38400`, `57600`, or `115200` to save a new baud rate and reboot. |
| `exit` | `passthrough` | In USB amplifier mode, close the debug console and return USB `Serial` to amplifier communications. In normal UART mode, this reports that the console remains active. |
| `setup` | `wifi-popup` | Open the hidden controller setup popup on the LCD. This is the same panel opened by tapping the top-left of the display. |
| `wifi-saved` | - | Print whether saved WiFi credentials exist, the saved SSID, and password length. The password itself is not printed. |
| `wifi-clear` | - | Clear saved WiFi credentials and schedule background reconnect retry state accordingly. |
| `poll on` | - | Enable periodic 2-second controller status lines on the debug console. |
| `poll off` | - | Disable periodic controller status lines. |

### Serial Diagnostics

The `serial` command prints amplifier-link counters:

- `transport`, `baud`, and `usb_console_active` - active amplifier transport, saved amplifier baud rate, and whether USB is currently reserved for the debug console.
- `rx_bytes` - bytes read from the active amplifier transport.
- `valid_packets` - valid 30-byte amplifier packets received.
- `invalid_checksums` - packets rejected by checksum.
- `max_available` - highest observed serial receive backlog on the active amplifier transport.
- `max_task_gap_ms` - longest observed gap between serial task service passes.
- `max_rx_byte_gap_us` - longest observed gap between received UART bytes.
- `max_drain_burst` - largest number of bytes drained in one service pass.
- `max_queue_depth` and `queued_packets_dropped` - decoded packet queue pressure between serial and UI/status publishing.
- `max_command_queue_depth` and `queued_commands_dropped` - outbound command queue pressure.
- `commands_sent` - amplifier commands transmitted.
- `last_checksum_error_ms` - `millis()` timestamp of the last checksum failure.
- `checksum_sync_resyncs` - checksum failures where the parser also detected sync bytes inside the bad packet.
- `last_bad_available`, `last_bad_packet_len`, `last_bad_checksum_expected`, and `last_bad_checksum_received` - context for the most recent checksum failure.

### Web Diagnostics

The `web` command prints HTTP server counters:

- `server_starts` - number of times the HTTP server was started after WiFi connection.
- `wifi_disconnects` - transitions from connected WiFi to disconnected.
- `clients` and `empty_requests` - accepted clients and clients that did not send a request line.
- `index`, `status`, `key`, and `logo` - request counters for `/`, `/api/status`, `/api/key?name=...`, and `/spe-logo.svg`.
- `bad_key_requests` - key endpoint requests with an unrecognised button name.
- `last_request_ms` and `path` - `millis()` timestamp and normalized path for the most recent request.

### Runtime Diagnostics

The `stats` command reports Mbed heap, stack, CPU, and RTOS thread data. Thread names currently include the main Arduino task plus the firmware's UI, console, WiFi, and amplifier serial tasks where the runtime exposes their names.

## Setup And WiFi

WiFi, display orientation, and amplifier serial transport are configured from a hidden setup popup on the LCD. Tap the top-left of the display or run the serial console `setup` command to open it.

Use `Search` to scan, select an SSID, enter the password, and press `Connect`. The popup also includes a display flip option plus amplifier serial transport and baud dropdowns. Changing the transport or baud saves the setting and reboots immediately.

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
