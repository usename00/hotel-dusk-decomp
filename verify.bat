@echo off
setlocal
chcp 65001 >nul
cd /d "%~dp0"
python "%~dp0tools\verify.py" %*
if errorlevel 1 (
    echo.
    echo [AUDIT FAILED]
    pause
    exit /b 1
)
echo.
pause
exit /b 0
