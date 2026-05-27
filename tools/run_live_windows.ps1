param(
    [string]$Config = "config\live_windows.json",
    [switch]$Build,
    [switch]$Diagnostics,
    [switch]$CalibrateAll
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location $projectRoot
try {
    if ($Build) {
        .\tools\build_debug.ps1
    }

    if ($Diagnostics) {
        .\build\debug\vision_camera_diagnostics.exe $Config
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }
    }

    if ($CalibrateAll) {
        .\build\debug\vision_live_calibrate_ui.exe $Config --all
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }
    }

    .\build\debug\vision_service.exe $Config
    exit $LASTEXITCODE
}
finally {
    Pop-Location
}
