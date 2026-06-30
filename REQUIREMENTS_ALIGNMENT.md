# Requirement Alignment Matrix

| Project requirement | Implementation status |
|---|---|
| Desktop app in C++/Qt | Implemented |
| Start menu | Implemented: ProjectStartDialog |
| New/open/recent projects | Implemented |
| Canvas dimensions / presets | Implemented in NewProjectDialog |
| Grid, snap, zoom, pan, coordinates | Implemented in CanvasWidget |
| Toolbar, library, properties, status bar | Implemented |
| Library categories | Implemented |
| Real-time search/filter | Implemented |
| Schematic preview | Implemented as preview text/pin description |
| Active devices list | Implemented |
| Place components | Implemented |
| Single selection | Implemented |
| Multi-select rectangle | Implemented |
| Drag & drop with snap | Implemented |
| Rotation and pin updates | Implemented |
| Mirroring | Implemented |
| Delete component/wire | Implemented |
| Properties window | Implemented |
| Pin hover detection | Implemented |
| 90-degree wiring | Implemented |
| Dot junction | Implemented as auto-detected wire crossing dots and netlist helper |
| Dragging wire with component | Implemented by rerouting connected wires |
| Basic sources/passives/interactives | Implemented graphically and partially in simulation |
| Logic gates + D flip-flop | Implemented |
| ADC/DAC | Implemented as ideal configurable components with Vref+/Vref-, bits and delay properties |
| Firmware HEX interface | Implemented in Microcontroller placeholder with Intel HEX validation/loading |
| LCD/keypad | Implemented graphically |
| Run/Pause/Stop/Step | Implemented |
| Wire animation/status | Simplified through LED/instrument state updates |
| Voltage probe, voltmeter, ammeter, oscilloscope | Implemented as simplified instruments |
| Save/Load JSON | Implemented |
| Undo/Redo | Implemented |
| Export image | Implemented |
| DRC: floating/short/logs | Implemented simplified checks |

## Honest gaps

Full analog numerical circuit analysis, accurate current/voltage solving across arbitrary resistor networks, real capacitor/inductor transient differential equations, and MCU instruction execution are not fully implemented.
