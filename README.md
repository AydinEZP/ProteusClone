# ProteusClone

ProteusClone is a Proteus-like desktop circuit simulator written in C++/Qt for an Object-Oriented Programming course project.

## Team Partition

The project is divided into three main implementation areas:

| Part | Owner | Scope |
|---|---|---|
| Part 1 | Aydin | UI, editor, canvas, component library, properties, project management, theme, help menu |
| Part 2 | Mohsen | Circuit core, pins, wires, junctions, DRC, analog/digital simulation, measurement tools, oscilloscope |
| Part 3 | Sepehr | Advanced components: MCU, HEX loader, RAM, ADC, DAC, LCD, Keypad, Seven Segment, advanced test circuits |

Full details are available in [`docs/TEAM_PARTITION.md`](docs/TEAM_PARTITION.md).

## Main Features

- Start menu and project creation flow
- Canvas with grid, snap, zoom, pan and coordinate display
- Component library, active device list and properties panel
- Component placement, selection, movement, rotation, mirroring and deletion
- Smart 90-degree wiring, pin detection and junction handling
- Mixed analog/digital simulation engine
- Run, Pause, Stop and Step controls
- DRC for blocking electrical faults, shorts and floating inputs
- Voltage probe, voltmeter, ammeter and oscilloscope
- ADC, DAC, MCU, External RAM, LCD, Keypad and Seven Segment
- Save/Load project files using `.pcj`
- Theme selection: System, Light and Dark
- Built-in English Help menu

## Build Requirements

- C++17 compiler
- CMake 3.16+
- Qt 6 Widgets

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/ProteusClone
```

On Windows, build with Qt Creator or CMake, then run `ProteusClone.exe`. If direct EXE launching fails because Qt DLLs are missing, run `deploy_windows_mingw.bat` after a successful build.

## Test Files

Sample `.pcj` circuits and `.hex` firmware files are available under `tests/`.

Recommended final checks:

- Basic placement, wiring and save/load tests
- Oscilloscope and measurement tests
- Switch, PushButton and Potentiometer behavior tests
- DRC blocking-fault tests
- MCU output-port test
- MCU input-mirror test
- RAM write/read test
- ADC and DAC tests

## Documentation

- [`docs/TEAM_PARTITION.md`](docs/TEAM_PARTITION.md)
- [`docs/COMMIT_HISTORY_PLAN.md`](docs/COMMIT_HISTORY_PLAN.md)
- [`docs/REQUIREMENT_MAPPING.md`](docs/REQUIREMENT_MAPPING.md)
- [`docs/USER_GUIDE.md`](docs/USER_GUIDE.md)
- [`docs/DEVELOPER_NOTES.md`](docs/DEVELOPER_NOTES.md)
- [`AI_USAGE.md`](AI_USAGE.md)
