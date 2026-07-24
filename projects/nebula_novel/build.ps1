Stop-Process -Name "nebula_novel" -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "  +==================================================+" -ForegroundColor Cyan
Write-Host "  |      Zenith Game SDK * Nebula Novel Builder      |" -ForegroundColor Cyan
Write-Host "  +==================================================+" -ForegroundColor Cyan
Write-Host ""

$Compiler = Join-Path $PSScriptRoot "..\..\zenith.exe"
$IncDir   = Join-Path $PSScriptRoot "..\..\include"
$SrcDir   = Join-Path $PSScriptRoot "..\..\src"

Write-Host "1. Rebuilding Zenith Compiler with MediaQuery semantics..." -ForegroundColor Yellow
& g++ -O3 -std=c++17 "$SrcDir\main.cpp" "$SrcDir\frontend\lexer.cpp" "$SrcDir\frontend\parser.cpp" "$SrcDir\frontend\semantic.cpp" "$SrcDir\frontend\formatter.cpp" "$SrcDir\lsp\lsp.cpp" "$SrcDir\backend\codegen.cpp" "$SrcDir\backend\js_codegen.cpp" "$SrcDir\backend\wasm_codegen.cpp" -I "$IncDir" -o "$Compiler" -lws2_32 -lwinhttp
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Zenith compiler build failed." -ForegroundColor Red
    exit 1
}

Write-Host "2. Transpiling scenes/main.zen to C++..." -ForegroundColor Yellow
& "$Compiler" scenes/main.zen -target cpp -o main.cpp
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Transpilation failed." -ForegroundColor Red
    exit 1
}

Write-Host "3. Compiling native executable..." -ForegroundColor Yellow
& g++ -O3 -std=c++17 main.cpp -I "$IncDir" -o nebula_novel.exe -lws2_32 -lwinhttp -lopengl32 -lgdi32 -luser32
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] C++ Compilation failed." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "  [OK] Build Successful! Output: nebula_novel.exe" -ForegroundColor Green
