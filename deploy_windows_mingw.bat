@echo off
setlocal EnableExtensions EnableDelayedExpansion

REM Deploy Qt DLLs and plugins next to ProteusClone.exe so it can be opened
REM directly by double-clicking. Run this AFTER a successful build.

set "ROOT=%~dp0"
cd /d "%ROOT%"

set "EXE="
set "MODE=release"

REM Common build locations inside the source folder
for %%P in (
  "build\Release\ProteusClone.exe"
  "build\Debug\ProteusClone.exe"
  "build\ProteusClone.exe"
  "out\build\x64-Debug\ProteusClone.exe"
  "out\build\x64-Release\ProteusClone.exe"
) do (
  if exist %%~P if not defined EXE (
    set "EXE=%%~P"
    echo %%~P | findstr /I "Debug" >nul && set "MODE=debug" || set "MODE=release"
  )
)

REM Qt Creator often builds outside the project directory. Try common sibling folders.
if not defined EXE (
  for /r "%ROOT%.." %%F in (ProteusClone.exe) do (
    echo %%F | findstr /I "build" >nul
    if not errorlevel 1 if not defined EXE (
      set "EXE=%%F"
      echo %%F | findstr /I "Debug" >nul && set "MODE=debug" || set "MODE=release"
    )
  )
)

if not defined EXE (
  echo Could not find ProteusClone.exe.
  echo Build the project first, then run this script again.
  echo If Qt Creator builds outside this folder, copy this .bat next to the build folder or run windeployqt manually.
  pause
  exit /b 1
)

set "WINDEPLOYQT="
for /f "delims=" %%i in ('where windeployqt.exe 2^>nul') do if not defined WINDEPLOYQT set "WINDEPLOYQT=%%i"
if not defined WINDEPLOYQT for /d %%Q in (C:\Qt\6.*\mingw_64\bin) do if exist "%%Q\windeployqt.exe" set "WINDEPLOYQT=%%Q\windeployqt.exe"
if not defined WINDEPLOYQT for /d %%Q in (C:\Qt\6.*\msvc*_64\bin) do if exist "%%Q\windeployqt.exe" set "WINDEPLOYQT=%%Q\windeployqt.exe"

if not defined WINDEPLOYQT (
  echo windeployqt.exe was not found.
  echo Check that Qt 6 MinGW was installed correctly.
  echo Usually it is here: C:\Qt\6.x.x\mingw_64\bin\windeployqt.exe
  pause
  exit /b 1
)

echo EXE:  %EXE%
echo Mode: %MODE%
echo Tool: %WINDEPLOYQT%
echo.

if /I "%MODE%"=="debug" (
  "%WINDEPLOYQT%" --debug --compiler-runtime "%EXE%"
) else (
  "%WINDEPLOYQT%" --release --compiler-runtime "%EXE%"
)

if errorlevel 1 (
  echo.
  echo windeployqt failed. The EXE may still run from Qt Creator.
  echo Try run_exe_with_qt_path.bat for emergency running.
  pause
  exit /b 1
)

echo.
echo Done. You can now double-click:
echo %EXE%
echo.
pause
