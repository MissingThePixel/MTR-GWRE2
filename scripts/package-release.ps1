param([string]$Executable = (Join-Path $PSScriptRoot '..\source\out\build\gw2_recompiled.exe'),
      [string]$RuntimeDirectory = (Join-Path $PSScriptRoot '..\sdk-install\bin'),
      [string]$OutputDirectory = (Join-Path $PSScriptRoot '..\dist'))
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
$package = Join-Path $OutputDirectory 'MTR-GWRE2-windows-x64'
$zip = Join-Path $OutputDirectory 'MTR-GWRE2-windows-x64.zip'
if ((Test-Path -LiteralPath $package) -or (Test-Path -LiteralPath $zip)) { throw 'Select an empty output directory; existing releases are not overwritten.' }
$binaries = @($Executable,(Join-Path $RuntimeDirectory 'rexruntime.dll'),(Join-Path $RuntimeDirectory 'rexgpu-xenos.dll'))
foreach ($file in $binaries) { if (!(Test-Path -LiteralPath $file)) { throw "Missing binary: $file" } }
New-Item -ItemType Directory -Path $package -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $package 'Game') -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $package 'LICENSES') -Force | Out-Null
Copy-Item -LiteralPath $Executable -Destination (Join-Path $package 'gw2_recompiled.exe')
foreach ($dll in @('rexruntime.dll','rexgpu-xenos.dll')) { Copy-Item -LiteralPath (Join-Path $RuntimeDirectory $dll) -Destination $package }
foreach ($name in @('run.bat','run-native.bat','run-options.bat','README.md')) { Copy-Item -LiteralPath (Join-Path $root $name) -Destination $package }
Copy-Item -LiteralPath (Join-Path $root 'LICENSE') -Destination (Join-Path $package 'LICENSE.txt')
foreach ($file in Get-ChildItem -LiteralPath (Join-Path $root 'LICENSES') -File) { Copy-Item -LiteralPath $file.FullName -Destination (Join-Path $package 'LICENSES') }
[IO.File]::WriteAllText((Join-Path $package 'Game\PLACE-GAME-FILES-HERE.txt'), 'Copy your complete extracted Geometry Wars: Retro Evolved 2 game here. This folder must contain default.xex and all accompanying assets.')
$allowed = @('gw2_recompiled.exe','rexruntime.dll','rexgpu-xenos.dll','run.bat','run-native.bat','run-options.bat','README.md','LICENSE.txt','Game\PLACE-GAME-FILES-HERE.txt')
foreach ($file in Get-ChildItem -LiteralPath $package -Recurse -File) {
    $relative = $file.FullName.Substring($package.Length+1)
    if ($relative -notin $allowed -and $relative -notmatch '^LICENSES\\[^\\]+\.txt$') { throw "Unexpected release file: $relative" }
}
Compress-Archive -LiteralPath $package -DestinationPath $zip -CompressionLevel Optimal
Get-FileHash -LiteralPath $zip -Algorithm SHA256
Write-Host "Release ZIP: $zip"
