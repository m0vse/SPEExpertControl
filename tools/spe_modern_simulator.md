# SPE Modern Protocol Simulator

This simulator emulates enough of the SPE Expert 1.3K/1.5K/2K serial protocol to develop parser support without a modern amplifier.

It uses real 1.5K-FA RCU LCD payload fixtures from `vu2cpl/macexpert-spe`:

- `tools/spe_modern_fixtures/*.bin`
- Each file is the 367-byte payload after `AA AA AA 6A`

Supported host commands:

- `0x80` RCU_ON: ACK and emit `AA AA AA 6A <fixture-payload>`
- `0x81` RCU_OFF: ACK and stop optional ticking
- `0x90` STATUS: emit a documented `0x43` 67-byte CSV status frame
- `0x01`-`0x11` key commands: ACK and rotate fixtures for basic UI movement testing

Run over TCP for parser development:

```powershell
python tools\spe_modern_simulator.py --tcp 127.0.0.1:9901 --ticker 0.5
```

Run against a USB-serial adapter connected to the controller UART selected for amp comms:

```powershell
python -m pip install pyserial
python tools\spe_modern_simulator.py --serial COM42 --baud 115200 --ticker 0.5
```

The simulator intentionally does not try to model amplifier RF behaviour. It is a wire-protocol fixture player with ACK/CSV responses.
