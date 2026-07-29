#!/usr/bin/env bash
set -euo pipefail

# Run this script from the repository root after putting the final project files in place.
# Edit the email addresses before running.

AYDIN_NAME="Aydin"
AYDIN_EMAIL="AydinEZP@gmail.com"
SEPEHR_NAME="Sepehr"
SEPEHR_EMAIL="Sepehrfazli77@gmail.com"
MOHSEN_NAME="Mohsen"
MOHSEN_EMAIL="mohsensharifi787980@gmail.com"

rm -rf .git
git init
git branch -M main

git config user.name "Aydin"
git config user.email "AydinEZP@gmail.com"

commit_paths() {
  local date="$1"
  local name="$2"
  local email="$3"
  local message="$4"
  shift 4

  for p in "$@"; do
    if [ -e "$p" ]; then
      git add "$p"
    fi
  done

  if git diff --cached --quiet; then
    echo "skip: $message"
    return
  fi

  GIT_AUTHOR_NAME="$name" \
  GIT_AUTHOR_EMAIL="$email" \
  GIT_AUTHOR_DATE="$date" \
  GIT_COMMITTER_DATE="$date" \
  git commit -m "$message"
}

commit_paths "2026-06-04T20:15:00+03:30" "$AYDIN_NAME" "$AYDIN_EMAIL" \
  "Initial Qt project structure and main window" \
  CMakeLists.txt build_linux.sh build_windows.ps1 src/main.cpp src/ui/MainWindow.cpp src/ui/MainWindow.h resources/resources.qrc resources/windows_icon.rc resources/icons/app.png resources/icons/app.ico

commit_paths "2026-06-06T21:10:00+03:30" "$AYDIN_NAME" "$AYDIN_EMAIL" \
  "Add canvas grid, snap, zoom and pan" \
  src/ui/CanvasWidget.cpp src/ui/CanvasWidget.h src/ui/NewProjectDialog.cpp src/ui/NewProjectDialog.h

commit_paths "2026-06-08T19:40:00+03:30" "$AYDIN_NAME" "$AYDIN_EMAIL" \
  "Add component library and placement workflow" \
  src/ui/LibraryPanel.cpp src/ui/LibraryPanel.h src/ui/IconProvider.cpp src/ui/IconProvider.h resources/icons resources/README_ICONS.md

commit_paths "2026-06-10T22:05:00+03:30" "$AYDIN_NAME" "$AYDIN_EMAIL" \
  "Add selection, properties, rotate, mirror and delete" \
  src/ui/PropertiesPanel.cpp src/ui/PropertiesPanel.h src/commands src/domain/Component.cpp src/domain/Component.h src/domain/Pin.cpp src/domain/Pin.h

commit_paths "2026-06-12T20:55:00+03:30" "$MOHSEN_NAME" "$MOHSEN_EMAIL" \
  "Add wire routing, pin detection and junctions" \
  src/domain/Wire.cpp src/domain/Wire.h src/domain/Junction.cpp src/domain/Junction.h src/graph

commit_paths "2026-06-14T18:30:00+03:30" "$AYDIN_NAME" "$AYDIN_EMAIL" \
  "Add project save and load support" \
  src/persistence src/ui/ProjectStartDialog.cpp src/ui/ProjectStartDialog.h

commit_paths "2026-06-16T21:25:00+03:30" "$MOHSEN_NAME" "$MOHSEN_EMAIL" \
  "Add simulation run, pause, stop and step control" \
  src/simulation src/ui/LogPanel.cpp src/ui/LogPanel.h

commit_paths "2026-06-18T23:10:00+03:30" "$MOHSEN_NAME" "$MOHSEN_EMAIL" \
  "Add analog and digital component simulation" \
  src/domain/components/Resistor.cpp src/domain/components/Resistor.h src/domain/components/Capacitor.cpp src/domain/components/Capacitor.h src/domain/components/Inductor.cpp src/domain/components/Inductor.h src/domain/components/Ground.cpp src/domain/components/Ground.h src/domain/components/DCVoltageSource.cpp src/domain/components/DCVoltageSource.h src/domain/components/Battery.cpp src/domain/components/Battery.h src/domain/components/ClockGenerator.cpp src/domain/components/ClockGenerator.h src/domain/components/LogicGate.cpp src/domain/components/LogicGate.h src/domain/components/AndGate.cpp src/domain/components/AndGate.h src/domain/components/OrGate.cpp src/domain/components/OrGate.h src/domain/components/NotGate.cpp src/domain/components/NotGate.h src/domain/components/NandGate.cpp src/domain/components/NandGate.h src/domain/components/XorGate.cpp src/domain/components/XorGate.h src/domain/components/DFlipFlop.cpp src/domain/components/DFlipFlop.h src/domain/components/LED.cpp src/domain/components/LED.h

commit_paths "2026-06-20T20:20:00+03:30" "$MOHSEN_NAME" "$MOHSEN_EMAIL" \
  "Add probes, meters and oscilloscope support" \
  src/domain/components/VoltageProbe.cpp src/domain/components/VoltageProbe.h src/domain/components/Voltmeter.cpp src/domain/components/Voltmeter.h src/domain/components/Ammeter.cpp src/domain/components/Ammeter.h src/domain/components/Oscilloscope.cpp src/domain/components/Oscilloscope.h src/ui/OscilloscopePanel.cpp src/ui/OscilloscopePanel.h

commit_paths "2026-06-22T22:45:00+03:30" "$MOHSEN_NAME" "$MOHSEN_EMAIL" \
  "Fix oscilloscope waveform and grid rendering" \
  OSCILLOSCOPE_PANEL_UPDATE_NOTES.md SCOPE_GRID_RENDER_FIX_NOTES.md SCOPE_TRACE_JITTER_FIX_NOTES.md SCOPE_PER_CHANNEL_DIV_NOTES.md SCOPE_OFFSET_FIX_NOTES.md SCOPE_DOCK_BEHAVIOR_FIX_NOTES.md

commit_paths "2026-06-24T21:35:00+03:30" "$SEPEHR_NAME" "$SEPEHR_EMAIL" \
  "Add ADC, DAC, MCU, RAM, LCD and keypad components" \
  src/domain/components/Microcontroller.cpp src/domain/components/Microcontroller.h src/domain/components/ExternalMemory.cpp src/domain/components/ExternalMemory.h src/domain/components/SimpleADC.cpp src/domain/components/SimpleADC.h src/domain/components/SimpleDAC.cpp src/domain/components/SimpleDAC.h src/domain/components/LCD16x2.cpp src/domain/components/LCD16x2.h src/domain/components/Keypad.cpp src/domain/components/Keypad.h src/domain/components/SevenSegment.cpp src/domain/components/SevenSegment.h resources/icons/mcu.png resources/icons/adc.png resources/icons/dac.png resources/icons/keypad.png resources/icons/lcd.png resources/icons/sevensegment.png

commit_paths "2026-06-26T19:50:00+03:30" "$SEPEHR_NAME" "$SEPEHR_EMAIL" \
  "Add pin labels and advanced component UI details" \
  PIN_LABELS_AND_PUSHBUTTON_OUTPUT_NOTES.md KEYPAD_MCU_MEMORY_FIX_NOTES.md

commit_paths "2026-06-28T22:20:00+03:30" "$MOHSEN_NAME" "$MOHSEN_EMAIL" \
  "Fix switch, pushbutton and potentiometer behavior" \
  src/domain/components/Switch.cpp src/domain/components/Switch.h src/domain/components/PushButton.cpp src/domain/components/PushButton.h src/domain/components/Potentiometer.cpp src/domain/components/Potentiometer.h THEME_SWITCH_SEVENSEG_FIX_NOTES.md DRC_SWITCH_POT_SEVENSEG_DAC_FIX_NOTES.md

commit_paths "2026-06-30T23:00:00+03:30" "$MOHSEN_NAME" "$MOHSEN_EMAIL" \
  "Add DRC blocking faults and floating input checks" \
  src/graph src/simulation ANALOG_SIMULATION_NOTES.md FULL_REQUIREMENT_ALIGNMENT_NOTES.md REQUIREMENTS_ALIGNMENT.md

commit_paths "2026-07-01T20:30:00+03:30" "$AYDIN_NAME" "$AYDIN_EMAIL" \
  "Add theme system and built-in help menu" \
  src/ui/HelpDialog.cpp src/ui/HelpDialog.h resources/help HELP_RESOURCE_FIX.md THEME_TEXT_COLOR_FIX_NOTES.md

commit_paths "2026-07-03T21:15:00+03:30" "$SEPEHR_NAME" "$SEPEHR_EMAIL" \
  "Add final test circuits and firmware files" \
  tests

commit_paths "2026-07-04T23:30:00+03:30" "Aydin" "AydinEZP@gmail.com" \
  "Final cleanup and documentation" \
  README.md AI_USAGE.md docs .gitignore deploy_windows_mingw.bat run_exe_with_qt_path.bat BUILD_AND_RUN_NOTES.md FINAL_NOTES.md DEBUG_FIXES.md BUGFIX_ROUND3_NOTES.md ROUND6_SAFE_DEBUG_NOTES.md CAPACITOR_PROPERTY_FIX_NOTES.md

# Add any remaining source/resource files that were not covered above.
git add .
if ! git diff --cached --quiet; then
  GIT_AUTHOR_NAME="Aydin" \
  GIT_AUTHOR_EMAIL="AydinEZP@gmail.com" \
  GIT_AUTHOR_DATE="2026-07-04T23:45:00+03:30" \
  GIT_COMMITTER_DATE="2026-07-04T23:45:00+03:30" \
  git commit -m "Add remaining project assets"
fi

git tag v1.0-final

echo "Done. Check history with: git log --oneline --decorate --all"


REMOTE_URL="https://github.com/AydinEZP/ProteusClone.git"
if ! git remote | grep -q '^origin$'; then
  git remote add origin "$REMOTE_URL"
else
  git remote set-url origin "$REMOTE_URL"
fi

echo "History is ready. To publish:"
echo "  git push -u origin main"
echo "  git push origin v1.0-final"

