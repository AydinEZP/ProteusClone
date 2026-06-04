cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
Write-Host "Build finished. For direct EXE execution, run deploy_windows_mingw.bat once."
