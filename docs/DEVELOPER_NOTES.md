# Developer Notes

## Architecture

- UI code is under `src/ui`.
- Component and device models are under `src/domain`.
- Netlist and connectivity are handled under `src/graph`.
- The simulation engine is under `src/simulation`.
- Project persistence is under `src/persistence`.

## Simulation Safety

Blocking DRC faults stop simulation before the solver continues. This prevents singular/ill-conditioned matrix errors from becoming repeated runtime failures.

## Advanced Components

MCU, RAM, ADC, DAC, LCD, Keypad and Seven Segment are implemented as component subclasses and participate in the same pin/net infrastructure as the base components.
