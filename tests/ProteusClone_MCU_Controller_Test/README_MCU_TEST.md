# MCU All-Ports LED CPU Test

This circuit is a standalone microcontroller test for ProteusClone.

Open `circuits/MCU_AllPorts_LED_CPU_Test.pcj`, then press Run.
The firmware is embedded in the `.pcj` file and also provided as `firmware/mcu_basic_all_ports.hex`.

## What it tests

- MCU VCC/GND wiring.
- 4 GPIO ports: P0, P1, P2, P3.
- Full-port output instructions: `MOV Pn,#imm` and `MOV Pn,A`.
- Internal RAM write/read: `MOV [addr],A` and `MOV A,[addr]`.
- Arithmetic: `ADD A,#imm`.
- Bit operations: `SETB Pn.bit` and `CLR Pn.bit`.
- Looping with `JMP`.

## Expected behavior

The 32 LEDs are grouped by port:

- Red row: P0.0 ... P0.7
- Blue row: P1.0 ... P1.7
- Green row: P2.0 ... P2.7
- Orange row: P3.0 ... P3.7

During Run, the rows repeatedly cycle through these visible phases:

1. P0 = 0x55, P1 = 0xAA, P2 = 0x0F, P3 = 0x42
   - P3=0x42 is produced by `0x12 + 0x30 = 0x42`, stored in internal RAM, then read back.
2. P0 = 0xAA, P1 = 0x55, P2 = 0xF0, P3 = 0x24
3. P0 is built with SETB into 0x89, P1 is cleared with CLR into 0xDB, P2=0x33, P3=0xCC
4. P2 and P3 show opposite walking-bit patterns.

## Firmware size

536 bytes
