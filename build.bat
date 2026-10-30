@echo off
setlocal
chcp 65001 >nul
cd /d "%~dp0"
python "%~dp0tools\build.py" %*
if errorlevel 1 (
    echo.
    echo [BUILD FAILED]
    pause
    exit /b 1
)
echo.
pause
exit /b 0
