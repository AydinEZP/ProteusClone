# MCU Input Pins Mirror Test

This test circuit checks MCU GPIO pins configured as real inputs.

## Files

- `circuits/MCU_Input_Pins_Mirror_To_Output_Test.pcj`
- `firmware/mcu_input_mirror.hex`

## What it tests

- `P0.0..P0.7` are configured as inputs.
- `P1.0..P1.7` are configured as outputs.
- `P2.0..P2.7` are configured as inputs.
- `P3.0..P3.7` are configured as outputs.
- The firmware continuously performs:

```text
P1 = P0
P3 = P2
```

## Expected behavior

1. Open `MCU_Input_Pins_Mirror_To_Output_Test.pcj`.
2. Press Run.
3. Hold any push button named `IN_P0.x`.
4. The matching LED named `OUT_P1.x` must turn on.
5. Release the button; the LED must turn off.
6. Hold any push button named `IN_P2.x`.
7. The matching LED named `OUT_P3.x` must turn on.
8. Release the button; the LED must turn off.

## Firmware bytes

```text
44 00 00    DIR P0,#00  ; P0 input
44 01 FF    DIR P1,#FF  ; P1 output
44 02 00    DIR P2,#00  ; P2 input
44 03 FF    DIR P3,#FF  ; P3 output
45 01 00    MOV P1,#00
45 03 00    MOV P3,#00
43 00       MOV A,P0
42 01       MOV P1,A
43 02       MOV A,P2
42 03       MOV P3,A
30 12 00    JMP 0x0012
```

The firmware is embedded in the `.pcj` file and is also provided as an Intel HEX file.
