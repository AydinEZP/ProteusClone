# Oscilloscope Dock Behavior Fix

- The Oscilloscope dock is hidden by default at application startup.
- The dock now has a close button (`QDockWidget::DockWidgetClosable`).
- A single click/select on an Oscilloscope component assigns that scope to the panel, reopens the dock, and raises its tab.
- Double-click keeps the same reopen behavior while still opening component properties.
- Other components do not force the oscilloscope panel open.
