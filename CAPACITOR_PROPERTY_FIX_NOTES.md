# Capacitor property fix

Fixed a property parsing bug where the PropertiesPanel displayed capacitance as strings such as `100 nF`, but `Capacitor::setProperty()` used `QString::toDouble()` directly. Qt returns 0 for strings with unit suffixes, so editing or leaving the field could silently set the capacitance to 0.

Changes:
- `Capacitor` now exposes `capacitance_nF` as a numeric editable field.
- `Capacitor::setProperty()` accepts both `capacitance_nF` and old `capacitance` keys.
- Accepted inputs include `100`, `100 nF`, `0.1 uF`, `1e-6 F`, and `470 pF`.
- Invalid text no longer overwrites the previous value with 0.
- Applied the same unit-safe fix to `Inductor` (`inductance_mH`) because it had the same suffix parsing bug.
