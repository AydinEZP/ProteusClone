# ProteusClone RAM / ADC / DAC Test Pack

Open each `.pcj` from `File > Open Project`, then press `Run`.

## 01_ram_write_read_mcu_external_memory.pcj
Tests MCU + ExternalMemory RAM write/read.

Firmware: `firmware/ram_write_read_a5.hex`

Expected behavior:
- MCU writes `0xA5` to external RAM address `0x10`.
- MCU then switches the data bus to input, asserts RAM read, reads `0xA5` back, and mirrors it to port `P3`.
- Voltage probes connected to P3 should show binary `1010 0101`:
  - P3.0 = HIGH
  - P3.1 = LOW
  - P3.2 = HIGH
  - P3.3 = LOW
  - P3.4 = LOW
  - P3.5 = HIGH
  - P3.6 = LOW
  - P3.7 = HIGH

## 02_adc_divider_to_dac_scope.pcj
Tests ADC + DAC loop with a known analog value.

Expected behavior:
- 10k/10k divider creates about `2.5 V`.
- 4-bit ADC should produce about code `8` for 2.5 V on a 0..5 V reference.
- DAC receives ADC bits and should output about `2.67 V` (`8/15 * 5`).
- Scope CH1 = DAC output, CH2 = ADC input.

## 03_dac_mcu_p3_ramp_scope.pcj
Tests DAC input pins driven by MCU digital output.

Firmware: `firmware/dac_ramp_p3.hex`

Expected behavior:
- MCU cycles P3 through `0x00`, `0x55`, `0xAA`, `0xFF`.
- DAC output should step between roughly `0 V`, `1.67 V`, `3.33 V`, `5 V`.
- Scope CH1 should show the stepped waveform.

## 04_combined_ram_adc_dac_smoke.pcj
Combined smoke test: RAM bus + ADC/DAC chain in one project.

Expected behavior:
- Loads all three advanced areas in one canvas.
- RAM part should still read back `0xA5` internally.
- ADC/DAC section should behave like test 02.

## Notes
- If the MCU does not auto-load by `firmwarePath` on your machine, the `.pcj` files also embed the firmware bytes in the component `flash` field.
- Delete the old build folder before testing a newly generated app build.
- If a test fails, report the exact file name and the Application Output log.
