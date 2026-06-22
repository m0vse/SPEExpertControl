<!--
  Modern SPE protocol testing notes.

  Copyright (C) 2026 Phil Taylor (M0VSE)
  SPDX-License-Identifier: GPL-3.0-only
-->

# Modern SPE Protocol Testing

The modern protocol path currently supports simulator-driven testing for the
SPE Expert 1.3K-FA, 1.5K-FA, and 2K-FA family. Real amplifier hardware is still
required before treating timing and screen refresh behaviour as final.

## Simulator Coverage

Run the simulator on the USB CDC port used for amplifier communications:

```powershell
python -u tools\spe_modern_simulator.py --serial COM27 --baud 115200 --sequence standby_idle,op_idle,setup_root,antenna_matrix,cat_settings,system_info,tun_ant_port
```

Verified by the simulator:

- Generic bootstrap sends the shared `0x80` RCU_ON probe.
- Modern `AA AA AA 6A ...` RCU frames are detected and routed to the modern runtime.
- The first detection frame is forwarded into the selected runtime instead of being discarded.
- The web API publishes `spe_modern` status with raw `40 x 8` LCD cells and attributes.
- Web button commands reach the simulator and can move setup/antenna highlights.
- The controller refreshes modern RCU screens with an `RCU_OFF`/`RCU_ON` cycle.

## RCU LCD Frame Notes

The modern `AA AA AA 6A` RCU screen frame is a fixture-style LCD snapshot, not
plain ASCII. The parser currently receives the 367-byte payload after the
`AA AA AA 6A` prefix.

Known layout from captured 1.5K-FA frames:

- Bytes `0..4`: frame metadata/preamble.
- Bytes `5..324`: 320 display cells, arranged as `8 rows x 40 columns`.
- Bytes `325..364`: 40 column attribute bytes used for row highlights.
- Bytes `365..366`: trailing checksum/status bytes from the captured payload.

The display cells are SPE LCD ROM character codes. They must be rendered with
the bitmap font table, not decoded as normal text. Text decoding is only a
diagnostic fallback.

Known custom cell codes:

- `0x8D`: horizontal rule segment.
- `0x8E`: horizontal/vertical rule junction.
- `0x8F`: vertical separator.
- `0xAA`: degree symbol.

Low character codes are offset by `0x20` for diagnostic text. For example,
`0x0D 0x0D 0x0E 0x0D 0x0D` represents the standby SWR value `--.--`.

The standby footer uses fixed separator cells. Simulator value patching must
only overwrite the value columns and must preserve the `0x8F` separator cells.
For the captured standby footer this means:

- IN value starts at row `7`, column `0`, width `2`.
- BAND value starts at row `7`, column `4`, width `4`.
- ANT value starts at row `7`, column `9`, width `2`.
- TEMP numeric value starts at row `7`, column `35`, width `3`.
- The degree/unit cells at columns `38..39` are part of the fixture and should
  not be overwritten by the simulator.

## Hardware Checks Still Required

Real amplifier testing should confirm:

- The default `SPE_MODERN_RCU_REFRESH_MS` value of `500 ms` is accepted by the amplifier.
- The amplifier tolerates repeated `RCU_OFF`/`RCU_ON` refreshes without flicker, missed screens, or sluggish controls.
- 115200 baud is reliable with the selected USB/UART transport and cable.
- Button commands are acknowledged quickly enough for the web busy indicator to clear at the right time.
- Screens that do not redraw automatically still update acceptably with the refresh cycle.
- The documented `0x90` CSV status response can be mixed with screen snapshots if live values need faster updates.

## Diagnostics

The serial console `serial` command includes modern refresh counters:

- `modern_refreshes`: number of scheduled modern RCU refresh cycles.
- `last_modern_refresh_ms`: controller uptime when the most recent refresh was queued.
- `commands_sent`: total amplifier command frames written to the selected transport.
- `last_command_opcode`: most recent opcode, useful for confirming `0x80`/`0x81` refresh cadence.

Use these counters during real hardware testing to compare command cadence with
screen response timing and any observed serial errors.
