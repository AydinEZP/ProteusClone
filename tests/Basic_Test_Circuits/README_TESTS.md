# ProteusClone .pcj Test Circuits

These project files are JSON-based `.pcj` files compatible with the current ProjectSerializer.
Open them from File > Open Project. Recommended test order:

1. `01_voltage_divider_meters.pcj` — Voltage divider: expected VM_mid ≈ 2.5 V, Scope CH1 ≈ 2.5 V, CH2 ≈ 5 V.
2. `02_rc_charge_scope.pcj` — RC transient: run/stop to test capacitor value, analog solver history, VM and Scope.
3. `03_clock_and_dff_logic.pcj` — Digital: AND output on CH1, DFF Q on CH2, LED follows Q after rising clock edge.
4. `04_adc_dac_loop.pcj` — ADC/DAC: divider feeds ADC VIN ≈ 2.5 V. 4-bit output drives DAC; VM_DAC should approach ≈ 2.67 V.
5. `05_junction_crossing_test.pcj` — Junction test: crossing at x≈310 has no junction; crossing at x≈650 has an explicit dot junction.
6. `06_rlc_transient_scope.pcj` — RLC transient: tests inductor/capacitor state-history and scope timebase.
7. `07_switch_pushbutton_interactive.pcj` — Interactive: toggle SW and hold BTN during Run; LEDs and scope channels should update.

Notes:
- Delete the old build/cache before testing a new application build.
- For transient/scope tests, press Run and then Stop to verify reset behavior.
- If a file fails to load, report the file name and the Application Output lines.
