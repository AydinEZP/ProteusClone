# ProteusClone .pcj Test Circuits

> Legacy documentation path: the eight files listed below are now available under `../Basic_Test_Circuits/circuits/`. This README is retained so existing references are not broken.

The project files use the JSON-based `.pcj` format supported by the current ProjectSerializer.
Open the canonical copies from File > Open Project. Recommended test order:

1. `../Basic_Test_Circuits/circuits/01_voltage_divider_meters.pcj` — Voltage divider: expected VM_mid ≈ 2.5 V, Scope CH1 ≈ 2.5 V, CH2 ≈ 5 V.
2. `../Basic_Test_Circuits/circuits/02_rc_charge_scope.pcj` — RC transient: run/stop to test capacitor value, analog solver history, VM and Scope.
3. `../Basic_Test_Circuits/circuits/03_clock_and_dff_logic.pcj` — Digital: AND output on CH1, DFF Q on CH2, LED follows Q after rising clock edge.
4. `../Basic_Test_Circuits/circuits/04_adc_dac_loop.pcj` — ADC/DAC: divider feeds ADC VIN ≈ 2.5 V. 4-bit output drives DAC; VM_DAC should approach ≈ 2.67 V.
5. `../Basic_Test_Circuits/circuits/05_junction_crossing_test.pcj` — Junction test: crossing at x≈310 has no junction; crossing at x≈650 has an explicit dot junction.
6. `../Basic_Test_Circuits/circuits/06_rlc_transient_scope.pcj` — RLC transient: tests inductor/capacitor state-history and scope timebase.
7. `../Basic_Test_Circuits/circuits/07_switch_pushbutton_interactive.pcj` — Interactive: toggle SW and hold BTN during Run; LEDs and scope channels should update.
8. `../Basic_Test_Circuits/circuits/08_mcu_lcd_keypad_scaffold.pcj` — MCU/LCD/Keypad scaffold: loads advanced components and bus wiring for UI/netlist stress testing.

Notes:
- Delete the old build/cache before testing a new application build.
- For transient/scope tests, press Run and then Stop to verify reset behavior.
- If a file fails to load, report the file name and the Application Output lines.
