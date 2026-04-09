param(
    [switch]$RunTests
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$vsDevCmd = "C:\BuildTools\Common7\Tools\VsDevCmd.bat"
$cmake = "C:\Users\pcben\tools\cmake\bin\cmake.exe"

if (-not (Test-Path $vsDevCmd)) {
    throw "VsDevCmd.bat est introuvable. Verifie l'installation de Build Tools."
}

if (-not (Test-Path $cmake)) {
    throw "cmake.exe est introuvable."
}

# On passe par l'environnement Visual Studio pour garantir la disponibilite de cl.exe, link.exe et rc.exe.
$commands = @(
    "call `"$vsDevCmd`" -host_arch=x64 -arch=x64 >nul",
    "`"$cmake`" --preset debug",
    "`"$cmake`" --build --preset debug"
)

if ($RunTests) {
    $commands += "`"$cmake`" --build --preset debug --target visiondarts_tests"
    $commands += "`"$cmake`" --build --preset debug --target vision_replay"
    $commands += "ctest --preset debug"
}

$commandLine = $commands -join " && "
Push-Location $projectRoot
try {
    cmd /c $commandLine
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}
finally {
    Pop-Location
}
