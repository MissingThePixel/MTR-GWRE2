param([Parameter(Mandatory=$true)][string]$GameDataRoot,
      [string]$SdkPrefix = (Join-Path $PSScriptRoot '..\sdk-install'), [int]$Jobs = 4)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$source = Join-Path $root 'source'
$GameDataRoot = (Resolve-Path -LiteralPath $GameDataRoot).Path
$SdkPrefix = (Resolve-Path -LiteralPath $SdkPrefix).Path
$xex = Join-Path $GameDataRoot 'default.xex'
if (!(Test-Path -LiteralPath $xex)) { throw 'The game folder must contain default.xex and accompanying assets.' }
$rexglue = Join-Path $SdkPrefix 'bin\rexglue.exe'
if (!(Test-Path -LiteralPath $rexglue)) { throw 'Run setup-sdk.ps1 first, or select a patched SDK with -SdkPrefix.' }
$manifest = Join-Path $source 'gw2_local.toml'
$content = [IO.File]::ReadAllText((Join-Path $source 'gw2_recompiled_manifest.toml'))
$gamePath = $GameDataRoot.Replace('\','/').Replace('"','\"')
$xexPath = $xex.Replace('\','/').Replace('"','\"')
$content = $content -replace 'game_root = "[^"]*"', ('game_root = "'+$gamePath+'"')
$content = $content -replace 'file_path = "[^"]*"', ('file_path = "'+$xexPath+'"')
[IO.File]::WriteAllText($manifest, $content, (New-Object Text.UTF8Encoding($false)))
& $rexglue codegen $manifest
if ($LASTEXITCODE -ne 0) { throw 'Code generation failed.' }
$build = Join-Path $source 'out\build'
& cmake -S $source -B $build -G Ninja -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$SdkPrefix" "-DMTR_GAME_MANIFEST=$manifest"
if ($LASTEXITCODE -ne 0) { throw 'Configuration failed. Use an x64 Visual Studio developer shell with Clang 20+.' }
& cmake --build $build -j $Jobs
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
Write-Host 'Built source/out/build/gw2_recompiled.exe. Run scripts/package-release.ps1 to create a player ZIP.'
