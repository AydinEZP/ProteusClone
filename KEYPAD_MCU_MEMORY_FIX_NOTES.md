# Keypad, MCU GPIO, and External Memory Fixes

## Interactive 4x4 keypad
- Keys can now be pressed directly on the schematic.
- Mouse press closes exactly one row/column contact.
- Mouse release opens the contact again.
- The pressed key is modeled as a real temporary net connection in `CircuitGraph::buildNetlist()`.
- `Ctrl+drag` moves the keypad without pressing a key.

## MCU GPIO expansion
- MCU GPIO expanded from 2 x 8-bit ports to 4 x 8-bit ports: P0, P1, P2, P3.
- Every GPIO bit now has a direction state:
  - 0 = input / high impedance
  - 1 = output / active driver
- Input pins are sampled from the connected circuit.
- Output pins alone are stamped as ideal digital drivers.
- New bytecode operations:
  - `0x42 port`        : MOV Pn,A
  - `0x43 port`        : MOV A,Pn
  - `0x44 port mask`   : DIR Pn,#mask (1=output, 0=input)
  - `0x45 port imm`    : MOV Pn,#imm
- Existing SETB/CLR instructions still work and automatically make the addressed bit an output.
- Old saved MCU projects remain load-compatible.

## External memory read/write bus
- `RD` and `WR` are active-low.
- Write cycle: `WR=0`, `RD=1`; memory samples D0..D7.
- Read cycle: `RD=0`, `WR=1`; memory actively drives D0..D7.
- Memory read data participates in:
  - digital net evaluation,
  - DRC driver-conflict checks,
  - analog MNA voltage-source stamping,
  - MCU GPIO input sampling.
- Simultaneous `RD=0` and `WR=0` is treated as a blocking memory-bus fault.

## Compatibility note
- New MCU instances start with GPIO in input/high-impedance mode.
- SETB/CLR and the full-port output opcodes configure the affected output bits automatically.
- Old serialized MCU files without direction fields keep the previous P0/P1 output behavior when loaded.
