# Oscilloscope grid rendering fix

This build fixes the oscilloscope display grid, not the schematic canvas grid.

Changes:
- Scope panel grid is now drawn in integer screen pixels.
- Anti-aliasing is disabled for grid lines only, then re-enabled for traces.
- Minor and major divisions are drawn separately.
- Center horizontal and vertical axes are fixed screen-space overlays.
- The compact oscilloscope component grid uses cosmetic pens so it does not shimmer under canvas zoom/pan.

This avoids the visible flicker/moire/jump that appeared on the oscilloscope grid while the trace was updating.
