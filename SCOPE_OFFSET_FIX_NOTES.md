# Scope Offset Sign Fix

Fixed vertical offset direction for the oscilloscope plot.

Before:
- Positive offset moved the trace downward on the screen.
- Negative offset moved the trace upward.

Now:
- Positive offset moves the trace upward.
- Negative offset moves the trace downward.

Changed files:
- `src/ui/OscilloscopePanel.cpp`
- `src/domain/components/Oscilloscope.cpp`

Implementation detail:
The drawing formulas now use `(sample + offset)` instead of `(sample - offset)` so the UI control behaves like a screen vertical-position knob.
