@echo off
setlocal
chcp 65001 >nul
cd /d "%~dp0"

echo ======================================================================
echo  Hotel Dusk: Room 215 (USA) - Native PC Port Builder (MSVC x64)
echo ======================================================================
echo.

if not exist "..\build" mkdir "..\build"

:: Check & auto-fetch SDL2 development libraries if missing
if not exist "%~dp0lib\SDL2-2.30.8\include\SDL.h" (
    echo [INFO] SDL2 SDK not found locally. Fetching official SDL2 VC package...
    powershell -Command "New-Item -ItemType Directory -Force -Path '%~dp0lib' | Out-Null; curl.exe -L -o '%~dp0lib\SDL2.zip' 'https://github.com/libsdl-org/SDL/releases/download/release-2.30.8/SDL2-devel-2.30.8-VC.zip'; Expand-Archive -Path '%~dp0lib\SDL2.zip' -DestinationPath '%~dp0lib' -Force; Remove-Item '%~dp0lib\SDL2.zip'"
    if not exist "%~dp0lib\SDL2-2.30.8\include\SDL.h" (
        echo [ERROR] Failed to fetch SDL2! Please check your internet connection.
        pause
        exit /b 1
    )
    echo [INFO] SDL2 package ready.
)

echo [1/3] Initializing Visual Studio 2022 x64 Build Environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 (
    echo [ERROR] Could not initialize MSVC 2022!
    pause
    exit /b 1
)

echo [2/3] Compiling Native C Source Files...
cl.exe /nologo /O2 /W3 /D_CRT_SECURE_NO_WARNINGS ^
    /I "%~dp0include" ^
    /I "%~dp0lib\SDL2-2.30.8\include" ^
    /I "%~dp0..\arm9\include" ^
    "%~dp0src\main.c" ^
    "%~dp0src\display.c" ^
    "%~dp0src\asset_loader.c" ^
    "%~dp0..\arm9\src\hotel_dusk_engine.c" ^
    /Fe:"%~dp0..\build\HotelDusk_PC.exe" ^
    /Fo:"%~dp0..\build\\" ^
    /link /LIBPATH:"%~dp0lib\SDL2-2.30.8\lib\x64" ^
    SDL2.lib SDL2main.lib shell32.lib user32.lib gdi32.lib /SUBSYSTEM:CONSOLE

if errorlevel 1 (
    echo.
    echo [BUILD ERROR] Compilation failed!
    pause
    exit /b 1
)

echo.
echo [3/3] Deploying SDL2.dll runtime...
copy /Y "%~dp0lib\SDL2-2.30.8\lib\x64\SDL2.dll" "%~dp0..\build\SDL2.dll" >nul

echo ======================================================================
echo  BUILD SUCCEEDED: HotelDusk_PC.exe is ready!
echo  Location: build\HotelDusk_PC.exe
echo ======================================================================
echo.
exit /b 0
