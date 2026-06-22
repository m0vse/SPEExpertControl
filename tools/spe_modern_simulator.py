#!/usr/bin/env python3
"""
SPE Expert 1.3K/1.5K/2K protocol simulator.

Copyright (C) 2026 Phil Taylor (M0VSE)
SPDX-License-Identifier: GPL-3.0-only
"""

from __future__ import annotations

import argparse
import pathlib
import socket
import sys
import time
from dataclasses import dataclass
from typing import Iterable


HOST_SYNC = b"\x55\x55\x55"
AMP_SYNC = b"\xAA\xAA\xAA"
RCU_FRAME_TYPE = 0x6A
STATUS_LEN = 0x43

CMD_RCU_ON = 0x80
CMD_RCU_OFF = 0x81
CMD_STATUS = 0x90

CMD_INPUT = 0x01
CMD_BAND_DOWN = 0x02
CMD_BAND_UP = 0x03
CMD_ANTENNA = 0x04
CMD_OFF = 0x0A
CMD_POWER = 0x0B
CMD_DISPLAY = 0x0C
CMD_OPERATE = 0x0D
CMD_CAT = 0x0E
CMD_LEFT = 0x0F
CMD_RIGHT = 0x10
CMD_SET = 0x11

LCD_BODY_OFFSET = 5
LCD_ROWS = 8
LCD_COLS = 40
LCD_ATTR_OFFSET = LCD_BODY_OFFSET + LCD_ROWS * LCD_COLS

BANDS = ["160m", "80m", "60m", "40m", "30m", "20m", "17m", "15m", "12m", "10m", "6m", "4m"]

SETUP_ITEMS = [
    ("CONFIG", "setup_root", 1, 1, 7),
    ("ANTENNA", "antenna_matrix", 2, 1, 7),
    ("CAT", "cat_settings", 3, 1, 3),
    ("MANUAL TUNE", "setup_root", 4, 1, 11),
    ("DISPLAY", "system_info", 1, 15, 7),
    ("BEEP", "setup_root", 2, 15, 10),
    ("START", "setup_root", 3, 15, 10),
    ("TEMP/FANS", "setup_root", 4, 15, 9),
    ("ALARMS LOG", "setup_root", 1, 29, 10),
    ("TUN ANT", "tun_ant_port", 2, 29, 7),
    ("RX ANT", "antenna_matrix", 3, 29, 7),
    ("EXIT", "standby_idle", 4, 29, 4),
]

ANTENNA_ITEMS = [
    (1, 8, 2), (2, 8, 1), (3, 8, 2), (4, 8, 1),
    (1, 11, 2), (2, 11, 2), (3, 11, 2), (4, 11, 2),
    (1, 21, 1), (2, 21, 1), (3, 21, 1), (4, 21, 1),
    (1, 24, 2), (2, 24, 2), (3, 24, 2), (4, 24, 2),
    (1, 33, 1), (2, 33, 1), (3, 33, 1),
    (1, 36, 2), (2, 36, 2), (3, 36, 2),
    (5, 34, 4),
]

TUN_ANT_ITEMS = [
    (1, 10, 7), (2, 10, 1), (3, 10, 1), (4, 10, 1),
    (1, 25, 4), (2, 25, 4), (3, 25, 4), (4, 25, 4),
    (1, 33, 5), (2, 33, 5), (3, 33, 5), (4, 33, 6),
]


@dataclass
class SimState:
    rcu_enabled: bool = False
    screen: str = "standby_idle"
    last_advance: float = 0.0
    operate: bool = False
    tx: bool = False
    power_watts: int = 0
    input_index: int = 1
    band_index: int = 8
    antenna_index: int = 2
    setup_cursor: int = 1
    antenna_cursor: int = 16
    tun_ant_cursor: int = 4


def checksum8(data: bytes) -> int:
    return sum(data) & 0xFF


def checksum16(data: bytes) -> int:
    return sum(data) & 0xFFFF


def build_host_ack(command: int) -> bytes:
    data = bytes([command])
    return AMP_SYNC + bytes([len(data)]) + data + bytes([checksum8(data)])


def build_csv_status(state: SimState, model_id: str = "15K") -> bytes:
    state.power_watts = 425 if state.tx else 0
    op = "O" if state.operate else "S"
    tx = "T" if state.tx else "R"
    power = f"{state.power_watts:04d}"
    vpa = "48.0" if state.operate else " 0.0"
    ipa = "18.5" if state.tx else " 0.0"

    fields = [
        "",
        model_id,
        op,
        tx,
        "A",
        "1",
        "05",
        "1a",
        "0r",
        "H",
        power,
        "1.10" if state.tx else "0.00",
        "1.05" if state.tx else "0.00",
        vpa.strip(),
        ipa.strip(),
        "33",
        "0",
        "0",
        "N",
        "N",
        "%^",
    ]
    csv = ",".join(fields).encode("ascii")
    if len(csv) > STATUS_LEN:
        raise ValueError(f"CSV status is {len(csv)} bytes, expected <= {STATUS_LEN}")
    csv = csv.ljust(STATUS_LEN, b" ")
    chk = checksum16(csv)
    return AMP_SYNC + bytes([STATUS_LEN]) + csv + bytes([chk & 0xFF, chk >> 8]) + b"\r\n"


def build_rcu_frame(payload: bytes) -> bytes:
    # 0x6A is a frame type, not a length. Real frames are delimited by the
    # next AA AA AA sync or by a receiver-side quiet-period timeout.
    return AMP_SYNC + bytes([RCU_FRAME_TYPE]) + payload


def patch_text(payload: bytearray, row: int, col: int, text: str) -> None:
    start = LCD_BODY_OFFSET + row * LCD_COLS + col
    for index, char in enumerate(text):
        pos = start + index
        if pos >= len(payload):
            break
        code = ord(char)
        if char == " ":
            payload[pos] = 0x00
        elif 0x21 <= code <= 0x7E:
            payload[pos] = code - 0x20


def clear_attrs(payload: bytearray) -> None:
    for col in range(LCD_COLS):
        pos = LCD_ATTR_OFFSET + col
        if pos < len(payload):
            payload[pos] = 0x00


def highlight(payload: bytearray, row: int, col: int, width: int) -> None:
    if row < 0 or row >= LCD_ROWS:
        return
    for index in range(max(0, width)):
        attr_col = col + index
        if 0 <= attr_col < LCD_COLS:
            pos = LCD_ATTR_OFFSET + attr_col
            if pos < len(payload):
                payload[pos] |= 1 << row


def fixture_payload(state: SimState, fixtures: dict[str, bytes]) -> tuple[str, bytes]:
    screen = state.screen
    if screen == "standby_idle" and state.operate:
        screen = "op_idle"
    payload = bytearray(fixtures.get(screen, fixtures["standby_idle"]))

    clear_attrs(payload)
    if screen == "setup_root":
        _, _, row, col, width = SETUP_ITEMS[state.setup_cursor % len(SETUP_ITEMS)]
        highlight(payload, row, col, width)
    elif screen == "antenna_matrix":
        row, col, width = ANTENNA_ITEMS[state.antenna_cursor % len(ANTENNA_ITEMS)]
        highlight(payload, row, col, width)
    elif screen == "tun_ant_port":
        row, col, width = TUN_ANT_ITEMS[state.tun_ant_cursor % len(TUN_ANT_ITEMS)]
        highlight(payload, row, col, width)

    patch_text(payload, 7, 0, f"{state.input_index:>2}")
    patch_text(payload, 7, 4, f"{BANDS[state.band_index % len(BANDS)]:>4}")
    patch_text(payload, 7, 9, f"{state.antenna_index:>2}")
    patch_text(payload, 7, 35, f"{32 + (2 if state.operate else 0):>3}")
    return screen, bytes(payload)


def parse_host_frames(buffer: bytearray) -> Iterable[bytes]:
    while True:
        start = buffer.find(HOST_SYNC)
        if start < 0:
            del buffer[:]
            return
        if start:
            del buffer[:start]
        if len(buffer) < 5:
            return
        count = buffer[3]
        total = 3 + 1 + count + 1
        if len(buffer) < total:
            return
        frame = bytes(buffer[:total])
        del buffer[:total]
        data = frame[4 : 4 + count]
        received = frame[-1]
        if checksum8(data) == received:
            yield data
        else:
            print(f"discarding bad host checksum: expected 0x{checksum8(data):02X}, got 0x{received:02X}", file=sys.stderr)


class Transport:
    def read(self, size: int = 256) -> bytes:
        raise NotImplementedError

    def write(self, data: bytes) -> None:
        raise NotImplementedError

    def close(self) -> None:
        pass


class TcpTransport(Transport):
    def __init__(self, host: str, port: int):
        self._server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._server.bind((host, port))
        self._server.listen(1)
        print(f"waiting for TCP client on {host}:{port}")
        self._conn, addr = self._server.accept()
        print(f"client connected from {addr[0]}:{addr[1]}")

    def read(self, size: int = 256) -> bytes:
        return self._conn.recv(size)

    def write(self, data: bytes) -> None:
        self._conn.sendall(data)

    def close(self) -> None:
        self._conn.close()
        self._server.close()


class SerialTransport(Transport):
    def __init__(self, port: str, baud: int):
        try:
            import serial  # type: ignore
        except ImportError as exc:
            raise SystemExit("pyserial is required for --serial. Install with: python -m pip install pyserial") from exc

        self._serial = serial.Serial(port=port, baudrate=baud, bytesize=8, parity="N", stopbits=1, timeout=0.1)
        print(f"opened serial simulator on {port} at {baud} baud")

    def read(self, size: int = 256) -> bytes:
        return self._serial.read(size)

    def write(self, data: bytes) -> None:
        self._serial.write(data)
        self._serial.flush()

    def close(self) -> None:
        self._serial.close()


def load_fixtures(path: pathlib.Path) -> list[tuple[str, bytes]]:
    fixtures = []
    for file in sorted(path.glob("*.bin")):
        data = file.read_bytes()
        if len(data) < 320:
            print(f"skipping short fixture {file.name}: {len(data)} bytes", file=sys.stderr)
            continue
        fixtures.append((file.stem, data))
    if not fixtures:
        raise SystemExit(f"no usable .bin fixtures found in {path}")
    return fixtures


def order_fixtures(fixtures: list[tuple[str, bytes]], preferred: list[str]) -> list[tuple[str, bytes]]:
    if not preferred:
        return fixtures

    by_name = {name: payload for name, payload in fixtures}
    ordered: list[tuple[str, bytes]] = []
    used: set[str] = set()
    for name in preferred:
        if name not in by_name:
            raise SystemExit(f"requested fixture '{name}' was not found")
        ordered.append((name, by_name[name]))
        used.add(name)

    for name, payload in fixtures:
        if name not in used:
            ordered.append((name, payload))
    return ordered


def handle_command(command: bytes, transport: Transport, state: SimState, fixtures: dict[str, bytes], model_id: str) -> None:
    if not command:
        return
    opcode = command[0]

    if opcode == CMD_RCU_ON:
        state.rcu_enabled = True
        transport.write(build_host_ack(opcode))
        name, payload = fixture_payload(state, fixtures)
        print(f"RCU_ON -> {name}.bin")
        transport.write(build_rcu_frame(payload))
        return

    if opcode == CMD_RCU_OFF:
        state.rcu_enabled = False
        transport.write(build_host_ack(opcode))
        print("RCU_OFF")
        return

    if opcode == CMD_STATUS:
        transport.write(build_csv_status(state, model_id=model_id))
        print("STATUS -> CSV")
        return

    transport.write(build_host_ack(opcode))
    if opcode == CMD_INPUT:
        state.input_index = 2 if state.input_index == 1 else 1
    elif opcode == CMD_BAND_DOWN:
        state.band_index = (state.band_index - 1) % len(BANDS)
    elif opcode == CMD_BAND_UP:
        state.band_index = (state.band_index + 1) % len(BANDS)
    elif opcode == CMD_ANTENNA:
        state.antenna_index = 1 + (state.antenna_index % 4)
    elif opcode == CMD_OPERATE:
        state.operate = not state.operate
        state.screen = "op_idle" if state.operate else "standby_idle"
    elif opcode == CMD_OFF:
        state.operate = False
        state.tx = False
        state.screen = "standby_idle"
    elif opcode == CMD_POWER:
        state.tx = not state.tx
    elif opcode == CMD_DISPLAY:
        state.screen = "system_info" if state.screen != "system_info" else ("op_idle" if state.operate else "standby_idle")
    elif opcode == CMD_CAT:
        state.screen = "cat_settings"
    elif opcode == CMD_LEFT:
        move_cursor(state, -1)
    elif opcode == CMD_RIGHT:
        move_cursor(state, 1)
    elif opcode == CMD_SET:
        press_set(state)

    print(f"KEY 0x{opcode:02X}")
    if state.rcu_enabled:
        name, payload = fixture_payload(state, fixtures)
        print(f"  -> {name}.bin")
        transport.write(build_rcu_frame(payload))


def move_cursor(state: SimState, delta: int) -> None:
    if state.screen == "setup_root":
        state.setup_cursor = (state.setup_cursor + delta) % len(SETUP_ITEMS)
    elif state.screen == "antenna_matrix":
        state.antenna_cursor = (state.antenna_cursor + delta) % len(ANTENNA_ITEMS)
    elif state.screen == "tun_ant_port":
        state.tun_ant_cursor = (state.tun_ant_cursor + delta) % len(TUN_ANT_ITEMS)
    else:
        state.screen = "setup_root"


def press_set(state: SimState) -> None:
    if state.screen == "setup_root":
        _, target, _, _, _ = SETUP_ITEMS[state.setup_cursor % len(SETUP_ITEMS)]
        state.screen = target
    elif state.screen in ("antenna_matrix", "tun_ant_port", "cat_settings", "system_info"):
        state.screen = "setup_root"
    else:
        state.screen = "setup_root"


def run(transport: Transport, fixtures: dict[str, bytes], cycle_order: list[str], model_id: str, ticker: float, cycle: float) -> None:
    state = SimState(last_advance=time.monotonic())
    pending = bytearray()
    last_tick = time.monotonic()

    try:
        while True:
            data = transport.read(256)
            if data:
                pending.extend(data)
                for command in parse_host_frames(pending):
                    handle_command(command, transport, state, fixtures, model_id)

            if ticker > 0 and state.rcu_enabled and time.monotonic() - last_tick >= ticker:
                name, payload = fixture_payload(state, fixtures)
                print(f"tick -> {name}.bin")
                transport.write(build_rcu_frame(payload))
                last_tick = time.monotonic()

            if cycle > 0 and state.rcu_enabled and time.monotonic() - state.last_advance >= cycle:
                if cycle_order:
                    try:
                        index = cycle_order.index(state.screen)
                    except ValueError:
                        index = -1
                    state.screen = cycle_order[(index + 1) % len(cycle_order)]
                    state.operate = state.screen == "op_idle"
                state.last_advance = time.monotonic()
                name, payload = fixture_payload(state, fixtures)
                print(f"cycle -> {name}.bin")
                transport.write(build_rcu_frame(payload))

            if not data:
                time.sleep(0.01)
    except (BrokenPipeError, ConnectionResetError, KeyboardInterrupt):
        print("simulator stopped")
    finally:
        transport.close()


def main() -> int:
    default_fixtures = pathlib.Path(__file__).resolve().parent / "spe_modern_fixtures"
    parser = argparse.ArgumentParser(description="Simulate an SPE modern amplifier serial protocol.")
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--tcp", metavar="HOST:PORT", help="listen for one TCP client")
    mode.add_argument("--serial", metavar="COMx", help="open a serial port using pyserial")
    parser.add_argument("--baud", type=int, default=115200, help="serial baud rate")
    parser.add_argument("--fixtures", type=pathlib.Path, default=default_fixtures, help="directory containing 367-byte RCU .bin payloads")
    parser.add_argument("--model", default="15K", choices=["13K", "15K", "20K"], help="CSV status model ID")
    parser.add_argument("--ticker", type=float, default=0.0, help="emit the current RCU fixture every N seconds while RCU is on")
    parser.add_argument("--cycle", type=float, default=0.0, help="advance to the next fixture every N seconds while RCU is on")
    parser.add_argument("--sequence", default="", help="comma-separated fixture names to put first, without .bin")
    args = parser.parse_args()

    fixtures = load_fixtures(args.fixtures)
    sequence = [name.strip() for name in args.sequence.split(",") if name.strip()]
    fixtures = order_fixtures(fixtures, sequence)
    fixture_map = dict(fixtures)
    cycle_order = [name for name, _ in fixtures]
    print(f"loaded {len(fixtures)} RCU fixtures from {args.fixtures}")

    if args.tcp:
        host, port_text = args.tcp.rsplit(":", 1)
        transport: Transport = TcpTransport(host, int(port_text))
    else:
        transport = SerialTransport(args.serial, args.baud)

    run(transport, fixture_map, cycle_order, args.model, args.ticker, args.cycle)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
