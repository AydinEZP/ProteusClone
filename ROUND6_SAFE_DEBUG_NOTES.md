# Round 6 safe debug based on Round 4

This package is based on `ProteusClone_round4_build_error_fix.zip`, not Round 5.

Changes are intentionally narrow:

- Preserved Round 4 selection/drag/delete/rotate/mirror behavior.
- Preserved Round 4 multi-bend wire drawing behavior.
- Preserved explicit-junction behavior: crossings do not auto-connect. Press `J` over a wire/crossing to place a junction dot.
- Added global GND semantics in the netlist: all Ground pins belong to the same 0V reference net.
- Added differential oscilloscope sampling: CH1 is measured relative to the oscilloscope GND pin.
- Updated probe/voltmeter/scope readings to use simplified net voltage instead of only LOW/HIGH booleans.
- Kept all icon PNGs and added a guaranteed generated fallback inside `IconProvider` so icons still appear if Qt resources are stale/missing.

Important: delete the old build folder before rebuilding.
