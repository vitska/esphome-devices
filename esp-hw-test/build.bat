@echo off
REM ESP8266 Hardware Test - Build and Flash Script for Windows

echo ============================================
echo ESP8266 D1 Mini Hardware Test
echo ============================================
echo.

REM Check if Docker is available
docker --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Docker is not installed or not running
    echo Please install Docker Desktop and try again
    exit /b 1
)

echo [1/3] Building Docker image...
docker build -t esp-hw-test .
if %errorlevel% neq 0 (
    echo [ERROR] Docker build failed
    exit /b 1
)

echo.
echo [2/3] Compiling firmware...
docker run --rm -v "%cd%":/project esp-hw-test pio run
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed
    exit /b 1
)

echo.
echo [3/3] Firmware compiled successfully!
echo.
echo Output: .pio\build\d1_mini\firmware.bin
echo.
echo To flash manually, run:
echo   pio run -t upload
echo.
echo Or use esptool directly:
echo   esptool.py --port COM8 write_flash 0x0 .pio\build\d1_mini\firmware.bin
echo.

pause
