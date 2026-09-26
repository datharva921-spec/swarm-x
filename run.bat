@echo off
echo ========================================================
echo               Launching SWARM-X Simulation
echo ========================================================

if not exist SWARM_X.exe (
    echo Executable not found. Compiling project...
    call build.bat
    if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
)

echo Starting SWARM-X HTTP Server on http://localhost:8080 ...
start "" http://localhost:8080
SWARM_X.exe
