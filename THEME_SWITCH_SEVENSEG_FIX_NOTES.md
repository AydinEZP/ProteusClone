# Theme, Ideal Switch, and Seven-Segment Pin Labels

## Theme
- Added View > Theme menu.
- Modes: System (Automatic), Light, Dark.
- Selection is persisted in QSettings under `appearance/theme`.
- System mode follows Qt/OS color scheme on Qt 6.5+ and falls back to startup system palette detection on older Qt 6 versions.

## Switch simulation
- Closed switch is modeled by exact net union in `CircuitGraph::buildNetlist()`.
- Open switch leaves A and B in separate nets.
- Removed the analog solver's large/small conductance approximation for Switch.
- Normal click toggles the switch; dragging still moves it.

## Seven Segment
- Added visible schematic labels for A, B, C, D, E, F, G, DP, and COM.
- Expanded bounding box so the pin row and labels are selectable/visible.
