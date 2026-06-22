# Oscilloscope Panel Update

This version upgrades the oscilloscope to match the project statement more closely.

## Implemented

- A dedicated dock panel named **Oscilloscope**.
- Full-size live plot separate from the schematic canvas.
- Two independent channels:
  - `CH1` pin, green trace
  - `CH2` pin, yellow trace
  - shared `GND` reference pin
- Live left-to-right time plotting while simulation is running.
- `Pause` stops sample acquisition because the simulation timer stops.
- `Stop` clears the trace memory and resets simulation time.
- Controls in the oscilloscope panel:
  - Channel enable/disable
  - Volt/Div
  - Time/Div
  - Vertical offset
  - Sample memory length
  - Clear trace
- Compact canvas preview also shows both channels.

## How to use

1. Add an Oscilloscope component from the Measurement category.
2. Wire `CH1` and/or `CH2` to the circuit nodes to inspect.
3. Wire `GND` to the desired reference node.
4. Click or double-click the oscilloscope component to bind it to the Oscilloscope dock panel.
5. Press Run. The plot updates live.

## Important limitation

The oscilloscope displays whatever voltages are produced by the current hybrid analog/digital simulator. If the analog solver does not model a specific component accurately yet, the scope will display that simplified result.
