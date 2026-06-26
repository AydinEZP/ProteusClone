# Pin labels and PushButton output update

- Added reusable pin-name rendering in `Component::drawPinLabels`.
- Added visible pin labels to ADC, DAC, MCU, External Memory, LCD, Keypad, DFF, logic gates, oscilloscope, and potentiometer symbols.
- PushButton is now a one-pin momentary digital source with pin `OUT`.
- Mouse hold on PushButton: `OUT = HIGH (5V)`. Mouse release: `OUT = LOW (0V)`.
- Ctrl+drag can be used to reposition PushButton without activating it.
- Removed the old two-terminal switch-like PushButton net-merging and conductance behavior.
- Simulation digital and analog paths both treat PushButton OUT as a driven source.
