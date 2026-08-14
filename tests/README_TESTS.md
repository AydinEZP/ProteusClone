# ProteusClone Manual Test Files

This directory contains Git-ready manual test fixtures for ProteusClone. The canonical suite has 22 `.pcj` circuits and 12 Intel HEX files. All paths below exist in this repository.

These are manual fixtures, not automated CTest targets. Build and run ProteusClone, open a `.pcj` through **File > Open Project**, optionally load its matching `.hex`, then use Run or Step and inspect the behavior described by the test group's README.

## Canonical PCJ circuits

- `Basic_Test_Circuits/circuits/01_voltage_divider_meters.pcj`
- `Basic_Test_Circuits/circuits/02_rc_charge_scope.pcj`
- `Basic_Test_Circuits/circuits/03_clock_and_dff_logic.pcj`
- `Basic_Test_Circuits/circuits/04_adc_dac_loop.pcj`
- `Basic_Test_Circuits/circuits/05_junction_crossing_test.pcj`
- `Basic_Test_Circuits/circuits/06_rlc_transient_scope.pcj`
- `Basic_Test_Circuits/circuits/07_switch_pushbutton_interactive.pcj`
- `Basic_Test_Circuits/circuits/08_mcu_lcd_keypad_scaffold.pcj`
- `MCU_Advanced_Tests/circuits/01_mcu_lcd_advanced.pcj`
- `MCU_Advanced_Tests/circuits/02_mcu_adc_dac_scope.pcj`
- `MCU_Advanced_Tests/circuits/03_mcu_external_ram_write.pcj`
- `MCU_Advanced_Tests/circuits/04_mcu_keypad_scan.pcj`
- `MCU_Controller_Test/circuits/MCU_AllPorts_LED_CPU_Test.pcj`
- `MCU_Input_Mirror_Test/circuits/MCU_Input_Pins_Mirror_To_Output_Test.pcj`
- `MCU_RAM_ADC_DAC_Tests/circuits/01_MCU_RAM_Write_Read.pcj`
- `MCU_RAM_ADC_DAC_Tests/circuits/02_MCU_ADC_Reader.pcj`
- `MCU_RAM_ADC_DAC_Tests/circuits/03_MCU_DAC_Ramp.pcj`
- `MCU_RAM_Proper_Test/circuits/MCU_RAM_Write_Read_4Pattern_Test.pcj`
- `RAM_ADC_DAC_Tests/circuits/01_ram_write_read_mcu_external_memory.pcj`
- `RAM_ADC_DAC_Tests/circuits/02_adc_divider_to_dac_scope.pcj`
- `RAM_ADC_DAC_Tests/circuits/03_dac_mcu_p3_ramp_scope.pcj`
- `RAM_ADC_DAC_Tests/circuits/04_combined_ram_adc_dac_smoke.pcj`

## Canonical Intel HEX firmware

- `MCU_Advanced_Tests/firmware/mcu_adc_dac_loop.hex`
- `MCU_Advanced_Tests/firmware/mcu_external_ram_write.hex`
- `MCU_Advanced_Tests/firmware/mcu_keypad_scan.hex`
- `MCU_Advanced_Tests/firmware/mcu_lcd_advanced.hex`
- `MCU_Controller_Test/firmware/mcu_basic_all_ports.hex`
- `MCU_Input_Mirror_Test/firmware/mcu_input_mirror.hex`
- `MCU_RAM_ADC_DAC_Tests/firmware/adc_mirror.hex`
- `MCU_RAM_ADC_DAC_Tests/firmware/dac_ramp.hex`
- `MCU_RAM_ADC_DAC_Tests/firmware/ram_write_read.hex`
- `MCU_RAM_Proper_Test/firmware/ram_full_write_read.hex`
- `RAM_ADC_DAC_Tests/firmware/dac_ramp_p3.hex`
- `RAM_ADC_DAC_Tests/firmware/ram_write_read_a5.hex`

## Preserved legacy copies

The existing `ProteusClone_MCU_Controller_Test`, `ProteusClone_MCU_Input_Mirror_Test` and `ProteusClone_MCU_RAM_Proper_Test` fixtures remain in place as valid legacy copies. The canonical RAM copy has a normalized firmware subdirectory path. The other `ProteusClone_*` directories retain earlier test-plan documentation and point to the canonical groups above. Use the canonical paths for final submission testing.
