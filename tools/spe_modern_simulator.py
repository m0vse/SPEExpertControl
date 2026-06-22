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


@dataclass
class SimState:
    rcu_enabled: bool = False
    fixture_index: int = 0
    operate: bool = False
    tx: bool = False
    power_watts: int = 0


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


def handle_command(command: bytes, transport: Transport, state: SimState, fixtures: list[tuple[str, bytes]], model_id: str) -> None:
    if not command:
        return
    opcode = command[0]

    if opcode == CMD_RCU_ON:
        state.rcu_enabled = True
        transport.write(build_host_ack(opcode))
        name, payload = fixtures[state.fixture_index % len(fixtures)]
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

    # Modern key commands are single byte 0x01-0x11. Keep this deliberately
    # simple: ACK them, alter a little state, and optionally advance fixtures.
    transport.write(build_host_ack(opcode))
    if opcode == 0x0D:
        state.operate = not state.operate
    elif opcode == 0x0A:
        state.operate = False
        state.tx = False
    elif opcode == 0x0C or opcode == 0x0E:
        state.fixture_index += 1
    elif opcode in (0x0F, 0x10, 0x11):
        state.fixture_index += 1

    print(f"KEY 0x{opcode:02X}")
    if state.rcu_enabled:
        name, payload = fixtures[state.fixture_index % len(fixtures)]
        print(f"  -> {name}.bin")
        transport.write(build_rcu_frame(payload))


def run(transport: Transport, fixtures: list[tuple[str, bytes]], model_id: str, ticker: float) -> None:
    state = SimState()
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
                name, payload = fixtures[state.fixture_index % len(fixtures)]
                print(f"tick -> {name}.bin")
                transport.write(build_rcu_frame(payload))
                last_tick = time.monotonic()

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
    args = parser.parse_args()

    fixtures = load_fixtures(args.fixtures)
    print(f"loaded {len(fixtures)} RCU fixtures from {args.fixtures}")

    if args.tcp:
        host, port_text = args.tcp.rsplit(":", 1)
        transport: Transport = TcpTransport(host, int(port_text))
    else:
        transport = SerialTransport(args.serial, args.baud)

    run(transport, fixtures, args.model, args.ticker)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
