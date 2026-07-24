@echo off
setlocal
if not exist "nebula_novel.exe" (
    echo Building project first...
    call build.bat
)
echo Launching Nebula Novel Game...
start nebula_novel.exe
endlocal
