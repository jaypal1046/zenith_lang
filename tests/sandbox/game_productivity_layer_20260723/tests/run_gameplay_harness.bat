@echo off
setlocal

echo.
echo   +==================================================+
echo   ^|      Zenith  *  Gameplay Harness Runner          ^|
echo   +==================================================+
echo.

g++ -O3 -std=c++17 gameplay_harness.cpp -I ../include -o gameplay_harness.exe -lwinhttp -lws2_32 -lopengl32 -lgdi32 -luser32
if %errorlevel% neq 0 (
    echo   [ERROR] Harness build failed.
    exit /b %errorlevel%
)

.\gameplay_harness.exe
endlocal
