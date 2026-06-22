# Scope Trace Jitter Fix

This patch fixes the one-frame horizontal jump observed in the oscilloscope trace.

Root causes addressed:

1. `clearSamples()` used to seed the scope buffer with future dummy timestamps. When the simulator restarted at `t = 0`, the first real sample was appended after samples with larger times, making the time array non-monotonic. The plot then used the last timestamp as `now`, which produced a visible jump.
2. `pushSample()` recalculated auto memory every tick from measured `dt`. Small timer jitter could resize/trim the buffer during live drawing.
3. Memory was only slightly larger than the visible window, so trimming could shift the left edge of the plotted time window.

Implemented fixes:

- `clearSamples()` now leaves the buffers empty instead of inserting seeded dummy samples.
- `pushSample()` enforces strictly increasing timestamps. If simulation time goes backwards, it clears the trace safely.
- Sample interval updates now have 5% hysteresis, ignoring tiny timing noise.
- Auto memory no longer shrinks during live sampling. It only grows automatically, and it can shrink only on explicit Time/Div/Clear/Load changes.
- Memory headroom was increased to 2.5x the visible time window to make trimming rare and stable.

Expected behavior:

- While running, the trace scrolls smoothly without a one-frame jump.
- Pause freezes the waveform.
- Stop/Clear resets the trace.
- Changing Time/Div may rescale the display intentionally, but normal ticks no longer shift the trace abruptly.
