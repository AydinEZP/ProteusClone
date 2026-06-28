# Electrical correctness and UI fix notes

- Potentiometer wiper now supports exact 0.0 and 1.0 end stops; endpoint topology is merged directly instead of stamping epsilon resistances.
- Open Switch is a true disconnection and remains FLOATING when isolated; closed Switch merges A/B nets directly.
- Blocking DRC runs before MNA and stops/blocks simulation for conflicting ideal drivers or floating required inputs.
- The analog solver rejects contradictory/redundant invalid source constraints safely instead of continuing into a singular matrix result.
- SevenSegment is a simulated common-cathode display; A..G are driven inputs, DP is optional, COM is an explicit required reference terminal.
- ADC and DAC symbols use spaced dynamic pin layouts. ADC no longer exposes an unused CLK pin; DAC input and reference labels are separated.
- The in-app English user guide documents these behaviors.
