# Team Partition

## Part 1 — UI / Editor / Project Management

**Owner:** Aydin

Scope:

- Start menu and new project flow
- Main window and dock layout
- Canvas UI
- Grid, snap, zoom and pan behavior
- Component library and active device list
- Properties panel
- Toolbar, menus and status bar
- Save/Load UI integration
- Theme system
- Built-in English Help menu

Relevant files:

- `src/ui/`
- `resources/`
- `src/persistence/`
- `src/commands/`
- `src/main.cpp`

## Part 2 — Circuit Core / Simulation

**Owner:** Mohsen

Scope:

- Base component/pin/wire/junction model
- Circuit graph and netlist construction
- Pin detection and wire connectivity
- Smart 90-degree wire routing
- Junction handling
- Analog simulation
- Digital simulation
- Run/Pause/Stop/Step simulation control
- DRC checks
- Blocking electrical fault detection
- Floating input detection
- Switch, PushButton and Potentiometer behavior
- Measurement tools
- Oscilloscope sampling and rendering behavior

Relevant files:

- `src/domain/`
- `src/graph/`
- `src/simulation/`
- `src/domain/components/Oscilloscope.*`
- `src/domain/components/Switch.*`
- `src/domain/components/PushButton.*`
- `src/domain/components/Potentiometer.*`
- `src/domain/components/VoltageProbe.*`
- `src/domain/components/Voltmeter.*`
- `src/domain/components/Ammeter.*`

## Part 3 — Advanced Components

**Owner:** Sepehr

Scope:

- Microcontroller model
- Intel HEX firmware loading
- MCU ports and pin directions
- MCU instruction execution
- External RAM write/read
- ADC
- DAC
- LCD 16x2
- Keypad matrix
- Seven Segment
- Pin labels for advanced components
- Advanced `.pcj` and `.hex` test circuits

Relevant files:

- `src/domain/components/Microcontroller.*`
- `src/domain/components/ExternalMemory.*`
- `src/domain/components/SimpleADC.*`
- `src/domain/components/SimpleDAC.*`
- `src/domain/components/LCD16x2.*`
- `src/domain/components/Keypad.*`
- `src/domain/components/SevenSegment.*`
- `tests/`
