@echo off
setlocal
cd /d "%~dp0"
where cmake >nul 2>nul
if errorlevel 1 (
  echo CMake bulunamadi. Visual Studio Installer icinde Desktop development with C++ ve CMake araclarini kurun.
  exit /b 1
)
cmake -S . -B build-windows -A x64
if errorlevel 1 exit /b 1
cmake --build build-windows --config Release
if errorlevel 1 exit /b 1
xcopy /e /i /y bin\previews build-windows\Release\previews >nul
if errorlevel 1 exit /b 1
ctest --test-dir build-windows -C Release --output-on-failure
if errorlevel 1 exit /b 1
echo Hazir: build-windows\Release\MasivaSkin.exe ve MasivaSkinBridge.dll (birlikte kullanin)
endlocal
