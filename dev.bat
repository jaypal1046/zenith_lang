@echo off
setlocal
echo.
echo   +========================================================+
echo   ^|     ZENITH ENGINE - HOT RELOAD & REFRESH WATCHER       ^|
echo   +========================================================+
echo.

set "TARGET_DIR=%1"
if "%TARGET_DIR%"=="" set "TARGET_DIR=."

py scripts/dev_watch.py "%TARGET_DIR%"

endlocal
