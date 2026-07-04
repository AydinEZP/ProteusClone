# Bugfix Round 3

Fixed based on reported issues:

- A4: improved direct EXE launching on Windows.
  - Added automatic CMake POST_BUILD deployment via `windeployqt` when available.
  - Rewrote `deploy_windows_mingw.bat` to detect Debug/Release EXE correctly.
  - Added `run_exe_with_qt_path.bat` as a fallback runner that temporarily adds Qt's bin directory to PATH.

- G8: crossing wires no longer automatically create electrical junctions.
  - Wire crossings are now graphical crossings only.
  - Electrical connection at a crossing requires an explicit junction dot.
  - Press `J` while the mouse is over a wire/crossing to create an explicit junction.

- Multi-bend wiring:
  - In Wire mode, click a start pin.
  - Click empty grid points to add as many 90-degree bend points as needed.
  - Click an end pin to complete the wire.
  - Manual bends are preserved when connected components move.

Recommended clean test:
1. Delete the old build folder.
2. Open `CMakeLists.txt` again in Qt Creator.
3. Configure, build, run.
4. For direct EXE launching, build once, then run `deploy_windows_mingw.bat`.
