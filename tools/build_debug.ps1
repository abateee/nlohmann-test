param(
    [switch]$RunTests
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$vsDevCmd = $env:VISIONDARTS_VSDEVCMD
$cmake = $env:VISIONDARTS_CMAKE

if (-not $env:VCPKG_ROOT) {
    $vcpkgCandidates = @(
        "C:\Users\pcben\tools\vcpkg",
        "C:\vcpkg"
    )
    $vcpkgRoot = $vcpkgCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if ($vcpkgRoot) {
        $env:VCPKG_ROOT = $vcpkgRoot
    }
}

if (-not $vsDevCmd) {
    $vsCandidates = @(
        "C:\BuildTools\Common7\Tools\VsDevCmd.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
    )
    $vsDevCmd = $vsCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}

if (-not $cmake) {
    $cmakeCommand = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($cmakeCommand) {
        $cmake = $cmakeCommand.Source
    }
}

if (-not (Test-Path $vsDevCmd)) {
    throw "VsDevCmd.bat est introuvable. Installe Visual Studio Build Tools 2022 ou definis VISIONDARTS_VSDEVCMD."
}

if (-not (Test-Path $cmake)) {
    throw "cmake.exe est introuvable. Installe CMake ou definis VISIONDARTS_CMAKE."
}

if (-not $env:VCPKG_ROOT -or -not (Test-Path $env:VCPKG_ROOT)) {
    throw "VCPKG_ROOT est introuvable. Installe vcpkg ou definis VCPKG_ROOT."
}

# On passe par l'environnement Visual Studio pour garantir la disponibilite de cl.exe, link.exe et rc.exe.
$commands = @(
    "call `"$vsDevCmd`" -host_arch=x64 -arch=x64 >nul",
    "set `"VCPKG_ROOT=$env:VCPKG_ROOT`"",
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
