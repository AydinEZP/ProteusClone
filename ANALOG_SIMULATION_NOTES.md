# Analog Simulation Patch Notes

This version changes `SimulationEngine::tick()` from a digital-only loop into a hybrid digital/analog loop:

1. Rebuild the netlist from pins, wires, junctions and closed switches/buttons.
2. Advance and evaluate digital elements first.
3. Treat digital outputs as fixed 0V/5V voltage sources.
4. Build and solve a simplified analog MNA matrix.
5. Feed solved voltages back into probes, voltmeters, ammeters, oscilloscopes, LEDs, ADC and DAC.

Implemented analog elements:

- `DCVoltageSource` and `Battery` as ideal voltage sources.
- `ClockGenerator`, `LogicGate`, `DFlipFlop`, `ADC` outputs and `DAC VOUT` as ideal voltage sources.
- `Resistor` as conductance between two nets.
- `Capacitor` with backward-Euler companion model and voltage history.
- `Inductor` with backward-Euler companion model and current history.
- `Ammeter` as a very small series resistance with current estimate.
- `LED` as a simplified threshold-based display/weak conductance model.

Limitations:

- This is not a full SPICE engine.
- Nonlinear diode solving is simplified.
- Ideal source conflicts are detected and only the first source is kept to avoid singular matrices.
- Floating analog nodes are weakly tied to ground for numerical stability.
