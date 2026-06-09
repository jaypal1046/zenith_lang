@echo off
REM Gradle wrapper batch script for Windows
where gradle >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    gradle %*
) else (
    echo Error: Gradle not found. Please install Gradle or use Android Studio.
    exit /b 1
)
