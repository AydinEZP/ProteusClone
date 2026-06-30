# Full Requirement Alignment Notes

This version is based on the last stable UI/scope/no-QPainterPath build and aligns the implementation more closely with the uploaded Proteus OOP specification.

## Implemented/Improved

- Logic gates now support a valid/undefined output state, editable propagation delay, and undefined input warnings.
- Digital outputs are converted into ideal 0V/5V sources before the analog MNA step.
- ADC and DAC now hold the last valid output during conversion delay, then apply the new code/voltage.
- Switch and PushButton are modeled as open/closed conductive elements in the analog solver.
- Added Potentiometer component with A/B/W pins and editable resistance/wiper ratio for live ADC tests.
- Added a minimal MCU model with Intel HEX loading, Flash, PC, ACC, RAM, ports, and simple MOV/ADD/JMP/SETB/CLR bytecode execution.
- Added ExternalMemory component with address/data/RD/WR pins and simple RAM behavior.
- LCD16x2 is no longer purely graphical: it accepts basic RS/RW/E/D0..D7 bus commands for clear, cursor, and character writes.
- Keypad is no longer purely graphical: it has a pressedKey property and row/column scan behavior.
- Save/Load includes the new components and preserves manual wire route state more accurately.
- Active Devices list now supports removing an item via right-click context menu.
- Save As now asks before overwriting an unrelated existing project file.

## Practical Scope

This remains an educational course-level simulator rather than a SPICE/Proteus clone. The analog engine uses a simplified MNA/backward-Euler solver. The MCU instruction set is intentionally minimal and documented in Microcontroller.cpp.
