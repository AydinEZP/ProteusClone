# ProteusClone Final GitHub Test Suite

This folder contains the final manual test fixtures for the ProteusClone OOP project.

## Important notes

- This package contains only test circuits, firmware and documentation; it does not change source code.
- The old scaffold-only file `08_mcu_lcd_keypad_scaffold.pcj` is intentionally excluded because it can trigger floating-input DRC errors. Use the dedicated LCD, Keypad and RAM tests instead.
- Keypad-to-LCD echo is not included here because that behavior requires a source-code integration/patch. The required keypad feature is covered by `04_mcu_keypad_scan.pcj`.
- For PushButton fixed-state demos, use Switch. PushButton is a momentary input.

## Required feature tests

| Feature | PCJ file | HEX file | Presentation action |
|---|---|---|---|
| Start/Open/Save/Library/Properties/Rotate/Mirror/Delete | `tests/Requirement_Coverage_Extra_PCJ/circuits/15_save_load_full_component_gallery.pcj` | `-` | Open file, select components, edit properties, rotate/mirror/delete, Save As and reopen. |
| Grid/Snap/Zoom/Pan/Coords | `manual inside editor + gallery file` | `-` | Use the editor toolbar/mouse on any PCJ; this does not need firmware. |
| Pin detection, 90-degree wiring, junction, crossing | `tests/Basic_Test_Circuits/circuits/05_junction_crossing_test.pcj` | `-` | Hover pins, draw/delete wires, show junction dots and crossings without dots. |
| DC source, GND, resistor, voltmeter, voltage probe | `tests/Basic_Test_Circuits/circuits/01_voltage_divider_meters.pcj` | `-` | Run and show divider voltage and meter/probe reading. |
| RC transient + oscilloscope + Run/Pause/Stop | `tests/Basic_Test_Circuits/circuits/02_rc_charge_scope.pcj` | `-` | Run, pause, stop; show scope waveform. |
| Clock + D flip-flop | `tests/Basic_Test_Circuits/circuits/03_clock_and_dff_logic.pcj` | `-` | Run/Step and show DFF output changing with clock. |
| ADC/DAC basic loop | `tests/Basic_Test_Circuits/circuits/04_adc_dac_loop.pcj` | `-` | Run and show analog-to-digital-to-analog loop. |
| RLC transient + scope | `tests/Basic_Test_Circuits/circuits/06_rlc_transient_scope.pcj` | `-` | Run and show transient response on scope. |
| Switch, PushButton, LED live interaction | `tests/Basic_Test_Circuits/circuits/07_switch_pushbutton_interactive.pcj` | `-` | Run, toggle switch and press button; show LED/wire state. |
| All logic gates | `tests/Requirement_Coverage_Extra_PCJ/circuits/09_all_logic_gates_showcase.pcj` | `-` | Show AND/OR/NOT/XOR/NAND outputs. |
| Battery, capacitor, inductor, passive gallery | `tests/Requirement_Coverage_Extra_PCJ/circuits/10_battery_passive_interactive_gallery.pcj` | `-` | Show source/passive components and properties. |
| Seven segment manual inputs | `tests/Requirement_Coverage_Extra_PCJ/circuits/11_sevensegment_manual_inputs.pcj` | `-` | Change inputs and show segment output. |
| DRC short circuit | `tests/Requirement_Coverage_Extra_PCJ/circuits/12_drc_short_vcc_to_gnd.pcj` | `-` | Press Run; simulation must be blocked with short-circuit log. |
| DRC floating input | `tests/Requirement_Coverage_Extra_PCJ/circuits/13_drc_floating_logic_input.pcj` | `-` | Press Run; simulation must be blocked with floating-input log. |
| DRC contended outputs | `tests/Requirement_Coverage_Extra_PCJ/circuits/14_drc_contended_outputs.pcj` | `-` | Press Run; simulation must be blocked with output-contention log. |
| MCU HEX loader, CPU step, port output | `tests/MCU_Controller_Test/circuits/MCU_AllPorts_LED_CPU_Test.pcj` | `tests/MCU_Controller_Test/firmware/mcu_basic_all_ports.hex` | Open, confirm HEX path, press Step repeatedly and show LED/port changes. |
| MCU input mirror | `tests/MCU_Input_Mirror_Test/circuits/MCU_Input_Pins_Mirror_To_Output_Test.pcj` | `tests/MCU_Input_Mirror_Test/firmware/mcu_input_mirror.hex` | Run, press input buttons, show mirrored output LEDs. |
| LCD 16x2 with MCU | `tests/MCU_Advanced_Tests/circuits/01_mcu_lcd_advanced.pcj` | `tests/MCU_Advanced_Tests/firmware/mcu_lcd_advanced.hex` | Run and show LCD command/text behavior. |
| MCU + ADC/DAC + scope | `tests/MCU_Advanced_Tests/circuits/02_mcu_adc_dac_scope.pcj` | `tests/MCU_Advanced_Tests/firmware/mcu_adc_dac_loop.hex` | Run and show MCU ADC read/DAC output/scope. |
| MCU external RAM write | `tests/MCU_Advanced_Tests/circuits/03_mcu_external_ram_write.pcj` | `tests/MCU_Advanced_Tests/firmware/mcu_external_ram_write.hex` | Run/Step and show RAM address/data/control bus. |
| MCU keypad scan | `tests/MCU_Advanced_Tests/circuits/04_mcu_keypad_scan.pcj` | `tests/MCU_Advanced_Tests/firmware/mcu_keypad_scan.hex` | Run, click keypad keys, explain row/column scan. This is not LCD echo. |
| MCU RAM write/read | `tests/MCU_RAM_ADC_DAC_Tests/circuits/01_MCU_RAM_Write_Read.pcj` | `tests/MCU_RAM_ADC_DAC_Tests/firmware/ram_write_read.hex` | Run/Step; show RAM write/read flow. |
| MCU ADC reader | `tests/MCU_RAM_ADC_DAC_Tests/circuits/02_MCU_ADC_Reader.pcj` | `tests/MCU_RAM_ADC_DAC_Tests/firmware/adc_mirror.hex` | Run; show ADC value read/mirrored. |
| MCU DAC ramp | `tests/MCU_RAM_ADC_DAC_Tests/circuits/03_MCU_DAC_Ramp.pcj` | `tests/MCU_RAM_ADC_DAC_Tests/firmware/dac_ramp.hex` | Run; show DAC output changing. |
| External RAM 4-pattern proper test | `tests/MCU_RAM_Proper_Test/circuits/MCU_RAM_Write_Read_4Pattern_Test.pcj` | `tests/MCU_RAM_Proper_Test/firmware/ram_full_write_read.hex` | Run/Step; show full RAM write/read pattern. |
| RAM A5 pattern test | `tests/RAM_ADC_DAC_Tests/circuits/01_ram_write_read_mcu_external_memory.pcj` | `tests/RAM_ADC_DAC_Tests/firmware/ram_write_read_a5.hex` | Run/Step; show external memory read/write. |
| ADC divider to DAC scope | `tests/RAM_ADC_DAC_Tests/circuits/02_adc_divider_to_dac_scope.pcj` | `-` | Run; show ADC/DAC/scope smoke test. |
| DAC P3 ramp scope | `tests/RAM_ADC_DAC_Tests/circuits/03_dac_mcu_p3_ramp_scope.pcj` | `tests/RAM_ADC_DAC_Tests/firmware/dac_ramp_p3.hex` | Run; show DAC waveform on scope. |
| Combined RAM/ADC/DAC smoke | `tests/RAM_ADC_DAC_Tests/circuits/04_combined_ram_adc_dac_smoke.pcj` | `-` | Open/run as combined advanced-component smoke test. |

## Recommended short demo order

1. `tests/Requirement_Coverage_Extra_PCJ/circuits/15_save_load_full_component_gallery.pcj`
2. `tests/Basic_Test_Circuits/circuits/05_junction_crossing_test.pcj`
3. `tests/Basic_Test_Circuits/circuits/01_voltage_divider_meters.pcj`
4. `tests/Basic_Test_Circuits/circuits/02_rc_charge_scope.pcj`
5. `tests/Basic_Test_Circuits/circuits/07_switch_pushbutton_interactive.pcj`
6. `tests/Requirement_Coverage_Extra_PCJ/circuits/09_all_logic_gates_showcase.pcj`
7. `tests/MCU_Controller_Test/circuits/MCU_AllPorts_LED_CPU_Test.pcj`
8. `tests/MCU_RAM_Proper_Test/circuits/MCU_RAM_Write_Read_4Pattern_Test.pcj`
9. `tests/MCU_Advanced_Tests/circuits/01_mcu_lcd_advanced.pcj`
10. `tests/MCU_Advanced_Tests/circuits/04_mcu_keypad_scan.pcj`
11. `tests/MCU_Advanced_Tests/circuits/02_mcu_adc_dac_scope.pcj`
12. Run the three DRC files: short, floating input, contended outputs.
