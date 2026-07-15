#requires -Version 5.1
<#
.SYNOPSIS
    Build wxl-converter.dll (P/Invoked by wxl-converter-ui).

.PARAMETER Config
    Build configuration. Default: Release.

.PARAMETER Arch
    Target architecture: x64 (default) or x86. x64 keeps the original "build/" output directory
    (unchanged from before -Arch existed) so existing local workflows and wxl-converter-ui's default
    DLL reference keep working with no flags; x86 uses a separate "build-x86/" directory so both
    architectures can coexist on disk (CI builds both in a matrix).

.PARAMETER Clean
    Delete the build directory before configuring.

.EXAMPLE
    .\build.ps1
.EXAMPLE
    .\build.ps1 -Arch x86
#>
param(
    [string]$Config = "Release",
    [ValidateSet("x64", "x86")]
    [string]$Arch = "x64",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

$root = $PSScriptRoot
$buildDir = if ($Arch -eq "x64") { Join-Path $root "build" } else { Join-Path $root "build-$Arch" }
$cmakeArch = if ($Arch -eq "x64") { "x64" } else { "Win32" }

if ($Clean -and (Test-Path $buildDir)) {
    Remove-Item $buildDir -Recurse -Force
}

if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

Write-Output "=== wxl-converter.dll ($Arch) ==="
cmake -S $root -B $buildDir -A $cmakeArch
cmake --build $buildDir --config $Config --parallel

$dll = Join-Path $buildDir "$Config\wxl-converter.dll"
if (Test-Path $dll) {
    Write-Output "OK -> $dll"
} else {
    Write-Error "Build did not produce $dll"
}
