# Debug fixes in this package

Fixed from the user report:

- Selection crash when clicking a component (`E1`): fixed `PropertiesPanel` iteration over a temporary `QMap`.
- Component selection/drag/delete (`E` group): hit-testing now uses raw world coordinates instead of snapped grid coordinates.
- Rotate/mirror keys (`F` group): focus and selection state are preserved after selecting a component.
- Wire connection (`G1/G2/G3/G5/G6`): wiring now supports both two-click and click-drag-release; duplicate and zero-length wires are blocked; deleting a wired component safely removes attached wires.
- Icons not visible: added generated fallback icons through `IconProvider`, so toolbar/library icons appear even if Qt resource loading is stale or the old build cache is used.
- Direct EXE launch: added `deploy_windows_mingw.bat` to run `windeployqt` and copy Qt DLLs/platform plugins next to the EXE.

Important: after extracting this package, delete the old build folder before rebuilding. Old CMake/Qt resource cache can keep the previous broken behavior.
