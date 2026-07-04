# Build / EXE notes

This version intentionally does **not** run `windeployqt` automatically after every build.
On some Windows setups, `windeployqt` fails at the POST_BUILD phase and Qt Creator reports:

```text
:-1: error: [CMakeFiles\ProteusClone.dir\build.make:1016: ProteusClone.exe] Error 1
```

That error is often caused by deployment, not by C++ compilation.

## Correct order

1. Delete the old `build` folder.
2. Open `CMakeLists.txt` in Qt Creator.
3. Configure with `Desktop Qt 6.x MinGW 64-bit`.
4. Build/Run from Qt Creator.
5. Only after a successful build, run:

```bat
deploy_windows_mingw.bat
```

Then the generated EXE can be opened directly by double-clicking.

## Emergency direct run

If direct EXE launch still fails, run:

```bat
run_exe_with_qt_path.bat
```

This temporarily adds Qt's `bin` folder to PATH and starts the EXE.
