# MCU + RAM / ADC / DAC test pack

These files are built for the current 4-port MCU model and the current `.pcj` JSON format.
Each circuit embeds the firmware bytes in the MCU component, so loading the `.pcj` is enough. The matching Intel HEX files are included as well.

## 1) 01_MCU_RAM_Write_Read.pcj
Purpose: verify **external RAM write and read through the MCU**.

Firmware sequence:
1. P1 drives address `0x12` to A0..A7.
2. P0 drives `0xA5` to D0..D7.
3. P2.0/P2.1 generate an active-low write cycle (`RD=1`, `WR=0`).
4. P0 changes to input mode.
5. P2 starts a read cycle (`RD=0`, `WR=1`).
6. MCU executes `MOV A,P0` and mirrors the value to P3.
7. P3 drives the Seven Segment raw segment inputs.

Expected read-back value: `0xA5`.
On the raw seven-segment pattern, bits 0, 2, 5 and 7 are ON: A, C, F and DP.

## 2) 02_MCU_ADC_Reader.pcj
Purpose: verify **Potentiometer -> ADC -> MCU**.

- Potentiometer W drives ADC VIN.
- ADC D0..D7 drive MCU P0.
- Firmware reads P0 and mirrors it to P1.
- P1 drives 8 LEDs, one per ADC bit.

The initial wiper is 0.5, so the expected ADC code is approximately `128` (`0x80`), therefore BIT7 should be ON and the lower bits should be OFF. Change the potentiometer `wiper` property toward `0.0` or `1.0` and observe the output code change.

## 3) 03_MCU_DAC_Ramp.pcj
Purpose: verify **MCU -> DAC -> Oscilloscope / Voltage Probe**.

- MCU P0 drives DAC D0..D7.
- Firmware adds `0x11` repeatedly and writes the accumulator to P0.
- DAC VOUT is connected to Scope CH1 and a Voltage Probe.
- Scope CH2 is tied to GND intentionally so DRC has no floating input.

Expected result: a stepped rising ramp from about 0 V to 5 V, wrapping around and repeating.

## Run procedure
1. Extract the whole ZIP while keeping `circuits/` and `firmware/` folders together.
2. Open one `.pcj` from the `circuits` folder.
3. Press Run.
4. For the DAC test, click the Oscilloscope component to open its panel.
5. For the ADC test, edit the potentiometer wiper value and observe the LED bits.

## Firmware instruction subset used
- `MOV A,#imm`
- `MOV Pn,A`
- `MOV A,Pn`
- `DIR Pn,#mask`
- `MOV Pn,#imm`
- `ADD A,#imm`
- `JMP addr`
- `NOP`
