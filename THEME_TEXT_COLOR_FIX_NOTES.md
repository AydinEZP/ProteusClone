# Theme text-color runtime fix

- Added an application-level theme stylesheet so text colors update immediately at runtime.
- Covered labels, menus, menu bar, dock widgets, status bar, buttons and common editors/views.
- Kept the palette update and added repainting of existing widgets after a theme change.
- Made the embedded Help browser and HTML guide theme-aware instead of forcing black text on a fixed white page.
- Circuit canvas rendering and electrical colors were not changed by this fix.
