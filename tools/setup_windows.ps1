param(
    [string]$VcpkgRoot = $env:VCPKG_ROOT,
    [string]$Triplet = "x64-windows",
    [switch]$InstallDeps
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot

function Find-VsDevCmd {
    if ($env:VISIONDARTS_VSDEVCMD -and (Test-Path $env:VISIONDARTS_VSDEVCMD)) {
        return $env:VISIONDARTS_VSDEVCMD
    }

    $candidates = @(
        "C:\BuildTools\Common7\Tools\VsDevCmd.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
    )
    return $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}

function Find-CMake {
    if ($env:VISIONDARTS_CMAKE -and (Test-Path $env:VISIONDARTS_CMAKE)) {
        return $env:VISIONDARTS_CMAKE
    }

    $command = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }
    return $null
}

$vsDevCmd = Find-VsDevCmd
$cmake = Find-CMake
$ninja = Get-Command ninja.exe -ErrorAction SilentlyContinue

Write-Host "Vision Darts - verification Windows"
Write-Host "Racine projet: $projectRoot"
Write-Host ""

if ($vsDevCmd) {
    Write-Host "[OK] Visual Studio Build Tools: $vsDevCmd"
} else {
    Write-Host "[KO] Visual Studio Build Tools introuvable"
}

if ($cmake) {
    Write-Host "[OK] CMake: $cmake"
} else {
    Write-Host "[KO] CMake introuvable"
}

if ($ninja) {
    Write-Host "[OK] Ninja: $($ninja.Source)"
} else {
    Write-Host "[KO] Ninja introuvable"
}

if (-not $VcpkgRoot) {
    $vcpkgCandidates = @(
        "C:\Users\pcben\tools\vcpkg",
        "C:\vcpkg"
    )
    $VcpkgRoot = $vcpkgCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}

if (-not $VcpkgRoot) {
    Write-Host "[KO] VCPKG_ROOT non defini et vcpkg introuvable dans les emplacements courants"
} elseif (Test-Path $VcpkgRoot) {
    Write-Host "[OK] vcpkg: $VcpkgRoot"
} else {
    Write-Host "[KO] VCPKG_ROOT pointe vers un dossier introuvable: $VcpkgRoot"
}

$installedDir = if ($VcpkgRoot) { Join-Path $VcpkgRoot "installed\$Triplet" } else { $null }
if ($installedDir -and (Test-Path $installedDir)) {
    Write-Host "[OK] vcpkg triplet: $installedDir"
} else {
    Write-Host "[KO] vcpkg triplet introuvable: $installedDir"
}

if ($InstallDeps) {
    if (-not $VcpkgRoot -or -not (Test-Path $VcpkgRoot)) {
        throw "Impossible d'installer les dependances: vcpkg introuvable."
    }

    $vcpkgExe = Join-Path $VcpkgRoot "vcpkg.exe"
    if (-not (Test-Path $vcpkgExe)) {
        throw "vcpkg.exe introuvable: $vcpkgExe"
    }

    Push-Location $projectRoot
    try {
        & $vcpkgExe install --triplet $Triplet
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }
    }
    finally {
        Pop-Location
    }
}

Write-Host ""
Write-Host "Variables utiles si les chemins ne sont pas standards:"
Write-Host '$env:VISIONDARTS_VSDEVCMD="C:\...\VsDevCmd.bat"'
Write-Host '$env:VISIONDARTS_CMAKE="C:\...\cmake.exe"'
Write-Host '$env:VCPKG_ROOT="C:\...\vcpkg"'
Write-Host ""
Write-Host "Commandes suivantes:"
Write-Host ".\tools\setup_windows.ps1 -InstallDeps"
Write-Host ".\tools\build_debug.ps1 -RunTests"
Write-Host ".\build\debug\vision_camera_diagnostics.exe config\live_windows.json"
