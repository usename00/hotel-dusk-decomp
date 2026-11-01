@echo off
setlocal
cd /d "%~dp0build"

if not exist "HotelDusk_PC.exe" (
    echo [INFO] HotelDusk_PC.exe not found. Building now...
    cd /d "%~dp0pc"
    call build_pc.bat
    cd /d "%~dp0build"
)

echo [LAUNCHING] Hotel Dusk: Room 215 - Native PC Port (Phase 2)...
start "" "%~dp0build\HotelDusk_PC.exe"
exit /b 0
