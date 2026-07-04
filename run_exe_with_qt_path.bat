@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "ROOT=%~dp0"
cd /d "%ROOT%"
set "EXE="

for %%P in (
  "build\Release\ProteusClone.exe"
  "build\Debug\ProteusClone.exe"
  "build\ProteusClone.exe"
  "out\build\x64-Debug\ProteusClone.exe"
  "out\build\x64-Release\ProteusClone.exe"
) do if exist %%~P if not defined EXE set "EXE=%%~P"

if not defined EXE (
  for /r "%ROOT%.." %%F in (ProteusClone.exe) do (
    echo %%F | findstr /I "build" >nul
    if not errorlevel 1 if not defined EXE set "EXE=%%F"
  )
)

if not defined EXE (
  echo Could not find ProteusClone.exe. Build first.
  pause
  exit /b 1
)

set "QT_BIN="
for /d %%Q in (C:\Qt\6.*\mingw_64\bin) do if exist "%%Q\Qt6Core.dll" set "QT_BIN=%%Q"
if not defined QT_BIN for /d %%Q in (C:\Qt\6.*\msvc*_64\bin) do if exist "%%Q\Qt6Core.dll" set "QT_BIN=%%Q"

if not defined QT_BIN (
  echo Could not find Qt bin folder.
  echo Expected something like C:\Qt\6.x.x\mingw_64\bin
  pause
  exit /b 1
)

set "PATH=%QT_BIN%;%PATH%"
echo Qt bin: %QT_BIN%
echo Running: %EXE%
echo.
"%EXE%"
pause
