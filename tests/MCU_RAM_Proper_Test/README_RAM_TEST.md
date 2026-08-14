# MCU + External RAM proper write/read test

Open `circuits/MCU_RAM_Write_Read_4Pattern_Test.pcj` and press Run.
The MCU program is embedded in the PCJ, so the circuit runs even if the external HEX path is not resolved. The matching HEX file is included for manual firmware loading.

## Wiring
- P0.0..P0.7 <-> RAM D0..D7 (bidirectional data bus)
- P1.0..P1.7 -> RAM A0..A7 (address bus)
- P2.0 -> RD (active LOW)
- P2.1 -> WR (active LOW)
- P3.0..P3.7 -> eight readback LEDs

## Test sequence
Firmware writes:
- RAM[0x12] = 0xA5
- RAM[0x34] = 0x3C
- RAM[0x56] = 0x5A
- RAM[0x78] = 0xC3

Then P0 is changed to INPUT and the MCU repeatedly reads those four addresses. Each read value is copied to P3 and shown on the 8-LED bank for about 0.45 seconds.

Expected repeating readback sequence:
`A5 -> 3C -> 5A -> C3 -> ...`

Bit order: READ_D0 is LSB and READ_D7 is MSB.

This test deliberately includes idle cycles between WR/RD state changes and releases the MCU data bus before RAM read cycles.
