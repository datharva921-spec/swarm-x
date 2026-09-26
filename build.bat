@echo off
echo ========================================================
echo               Building SWARM-X Simulation Engine
echo ========================================================

g++ -std=c++14 -O2 -I"include" main.cpp src\*.cpp -lws2_32 -lwsock32 -o SWARM_X.exe

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Build completed successfully: SWARM_X.exe generated.
) else (
    echo [ERROR] Compilation failed. Please check error messages above.
    pause
    exit /b %ERRORLEVEL%
)
