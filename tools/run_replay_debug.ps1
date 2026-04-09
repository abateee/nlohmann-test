param(
    [string]$ScenarioPath = "fixtures",
    [string]$DebugOutput = "build/debug_output"
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$binary = Join-Path $projectRoot "build\debug\vision_replay.exe"

if (-not (Test-Path $binary)) {
    throw "vision_replay.exe est introuvable. Lance d'abord tools/build_debug.ps1."
}

Push-Location $projectRoot
try {
    & $binary $ScenarioPath --debug-out $DebugOutput
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}
finally {
    Pop-Location
}
