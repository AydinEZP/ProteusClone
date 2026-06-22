# Oscilloscope per-channel division update

This build updates the oscilloscope so vertical divisions are independent per channel.

## What changed

- CH1 has its own `Volt/Div` setting.
- CH2 has its own `Volt/Div` setting.
- CH1 has its own vertical offset.
- CH2 has its own vertical offset.
- `Time/Div` remains global, as it is the common horizontal timebase of the oscilloscope.
- Old projects that use `voltsPerDiv`, `verticalOffset`, or `scale` still load; legacy values are applied to both channels.

## Updated classes

- `domain/components/Oscilloscope.h`
- `domain/components/Oscilloscope.cpp`
- `ui/OscilloscopePanel.h`
- `ui/OscilloscopePanel.cpp`

## New/updated properties

- `ch1_volts_per_div`
- `ch2_volts_per_div`
- `ch1_vertical_offset_v`
- `ch2_vertical_offset_v`
- `time_per_div_s`
- `memory_auto`

## UI behavior

The Oscilloscope dock now shows separate controls:

- CH1 Volt/Div
- CH2 Volt/Div
- CH1 offset
- CH2 offset
- Time/Div
- Auto memory summary

The plot renders CH1 and CH2 with their own scale and offset while using the same timebase.
