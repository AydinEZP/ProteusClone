# MCU Advanced Test Pack

This pack is built for the current ProteusClone educational MCU model.
Each `.pcj` embeds the firmware bytes, so the circuit can run even if the HEX path is not resolved. The matching Intel HEX file is also included separately for testing the firmware loader.

## 1) 01_mcu_lcd_advanced.pcj
Firmware: `firmware/mcu_lcd_advanced.hex`

Expected behavior:
- LCD is cleared and then displays: `ADVANCED OK`
- Green heartbeat LED connected to MCU P1.3 blinks continuously.
- Firmware uses MOV immediate, ADD immediate, RAM store, RAM load, SETB, CLR and JMP.
- ACC should become 8 during the initial CPU smoke test.

## 2) 02_mcu_adc_dac_scope.pcj
Firmware: `firmware/mcu_adc_dac_loop.hex`

Signal path:
`MCU P0[7:0] -> DAC8 -> analog VOUT -> ADC8 -> Seven Segment raw segment pins`

Expected behavior:
- MCU outputs a repeating digital staircase code.
- DAC VOUT follows the code with conversion delay.
- Oscilloscope CH1 shows the DAC staircase.
- Oscilloscope CH2 shows MCU P0.7.
- ADC converts DAC VOUT back to an 8-bit code.
- ADC D0..D7 directly drive A,B,C,D,E,F,G,DP of the seven-segment. This is a raw bit visualization, not a BCD digit decoder.

## 3) 03_mcu_external_ram_write.pcj
Firmware: `firmware/mcu_external_ram_write.hex`

Bus mapping:
- MCU P0[7:0] -> RAM D0..D7
- MCU P1[7:0] -> RAM A0..A7
- RD is tied HIGH (memory does not drive the data bus)
- WR is tied LOW (write mode)

The firmware repeatedly writes:
- address 0x10 <- 0xA5
- address 0x11 <- 0x3C
- address 0x12 <- 0x5A
- address 0x13 <- 0xC3

After running, save the project and inspect the serialized `ram` array if you want to verify the written cells numerically.

## 4) 04_mcu_keypad_scan.pcj
Firmware: `firmware/mcu_keypad_scan.hex`

Expected behavior:
- MCU P1.4..P1.7 scans keypad rows R1..R4, one LOW row at a time.
- The keypad has `pressedKey = 5` in the project file.
- C1..C4 have 10 kOhm pull-ups and voltage probes.
- COL2 should pulse LOW when row R2 is scanned; the other columns remain HIGH.

Note: the current educational MCU bytecode model has output-style port pins and no dedicated input-read opcode. Therefore this circuit validates keypad row/column scan behavior electrically and visually, but does not branch the MCU firmware based on the pressed key.

## How to test
1. Open a `.pcj` file in ProteusClone.
2. Press Run.
3. For the LCD test, wait for the message to be written one character at a time.
4. For the DAC/ADC test, open the oscilloscope panel and observe CH1/CH2.
5. For the RAM test, run for a few seconds, Stop, Save, then inspect RAM serialization if needed.
6. For the keypad test, open Properties for the Keypad and change `pressedKey` to another key (for example `1`, `5`, `9`, `D`) to validate different row/column combinations.

## MCU bytecode used by these tests
- `00` NOP
- `10 imm` MOV A,#imm
- `11 addr` MOV [addr],A
- `12 addr` MOV A,[addr]
- `20 imm` ADD A,#imm
- `30 lo hi` JMP addr
- `40 pb` SETB Pn.bit
- `41 pb` CLR Pn.bit

Port-byte encoding for SETB/CLR: high nibble selects port (0 or 1), low three bits select bit 0..7.
