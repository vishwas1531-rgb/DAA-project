@echo off
TITLE Public Transport Crowd Estimator - PMPML DAA Project
echo ===================================================
echo   PMPML Public Transport Crowd Estimator Launcher
echo ===================================================
echo.

cd /d "%~dp0"

echo [1/4] Checking Python Installation...
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Python is not installed or not added to PATH.
    pause
    exit /b 1
)

echo [2/4] Checking C++ Compiler (g++)...
g++ --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] MinGW g++ compiler is not found in PATH.
    pause
    exit /b 1
)

echo [3/4] Compiling C++ DAA Engine...
g++ -std=c++17 -O2 src/main.cpp src/algorithms.cpp src/data_manager.cpp -o engine.exe
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed! Please check C++ code.
    pause
    exit /b 1
)
echo C++ Engine compiled successfully (engine.exe).

echo [4/4] Installing Python Dependencies and Starting Server...
pip install -r requirements.txt >nul 2>&1
python app.py

pause