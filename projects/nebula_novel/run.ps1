if (-not (Test-Path "nebula_novel.exe")) {
    Write-Host "Building project first..." -ForegroundColor Yellow
    & ".\build.ps1"
}
Write-Host "Launching Nebula Novel Game..." -ForegroundColor Green
Start-Process ".\nebula_novel.exe"
