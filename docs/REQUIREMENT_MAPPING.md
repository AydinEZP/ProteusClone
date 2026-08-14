# Requirement Mapping

| Requirement Area | Implementation |
|---|---|
| Start menu | `ProjectStartDialog`, new/open/recent project flow |
| Canvas and design environment | `CanvasWidget`, grid, snap, zoom, pan, coordinates |
| Component library | `LibraryPanel`, component categories, search, icons and placement |
| Component editing | selection, multi-selection, movement, rotate, mirror, delete and properties |
| Wiring and connections | pins, wires, 90-degree routing, junctions and dynamic wire updates |
| Base components | resistor, capacitor, inductor, GND, DC source, battery, clock, switch, pushbutton, LED and logic gates |
| Sequential/digital logic | D Flip-Flop, clock generator, HIGH/LOW/Undefined handling |
| Advanced components | MCU, ADC, DAC, External RAM, LCD, Keypad and Seven Segment |
| Simulation control | Run, Pause, Stop, Step and simulation time handling |
| DRC | blocking short-circuit/floating-input checks and simulation abort on critical faults |
| Measurement tools | voltage probe, voltmeter, ammeter and oscilloscope |
| Oscilloscope | 2 channels, per-channel volt/div, time/div, memory, pause/stop behavior and close/reopen dock behavior |
| Save/Load | `.pcj` project serializer |
| UI polish | theme selection, status bar, icons and built-in English Help menu |
| Test assets | 22 canonical manual `.pcj` circuits and 12 Intel HEX firmware files in `tests/`, covering basic, measurement, RAM, ADC/DAC and MCU/advanced scenarios |
