param([string]$SdkDirectory = (Join-Path $PSScriptRoot 'thirdparty\rexglue-sdk'), [int]$Jobs = 4)
$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
$SdkDirectory = [IO.Path]::GetFullPath($SdkDirectory)
$pin = 'f5337cdc947ff6d4c4196737e2c807a48f2a1fc2'
function Run-Git([string[]]$GitArguments) {
    & git -c core.longpaths=true @GitArguments
    if ($LASTEXITCODE -ne 0) { throw 'Git operation failed.' }
}
if (!(Test-Path -LiteralPath (Join-Path $SdkDirectory '.git'))) {
    Run-Git -GitArguments @('clone','--branch','v0.10.0','--depth','1','https://github.com/rexglue/rexglue-sdk.git',$SdkDirectory)
}
$head = (& git -C $SdkDirectory rev-parse HEAD).Trim()
if ($head -ne $pin) { throw "Expected ReXGlue v0.10.0 ($pin), found $head. Use a separate SDK clone." }
Run-Git -GitArguments @('-C',$SdkDirectory,'submodule','update','--init','--recursive')
function Apply-Patch([string]$Directory, [string]$Patch) {
    & git -C $Directory apply --check --reverse $Patch 2>$null
    if ($LASTEXITCODE -eq 0) { return }
    Run-Git -GitArguments @('-C',$Directory,'apply','--check',$Patch)
    Run-Git -GitArguments @('-C',$Directory,'apply',$Patch)
}
Apply-Patch $SdkDirectory (Join-Path $projectRoot 'patches\rexglue-v0.10.0.patch')
Apply-Patch (Join-Path $SdkDirectory 'thirdparty\libmspack') (Join-Path $projectRoot 'patches\libmspack-windows.patch')
$overlay = Join-Path $projectRoot 'sdk-overrides'
foreach ($file in Get-ChildItem -LiteralPath $overlay -Recurse -File) {
    $relative = $file.FullName.Substring($overlay.Length + 1)
    $destination = Join-Path $SdkDirectory $relative
    New-Item -ItemType Directory -Path (Split-Path $destination -Parent) -Force | Out-Null
    Copy-Item -LiteralPath $file.FullName -Destination $destination -Force
}
$build = Join-Path $SdkDirectory 'out\build\mtr-release'
$prefix = Join-Path $projectRoot 'sdk-install'
& cmake -S $SdkDirectory -B $build -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Release "-DCMAKE_INSTALL_PREFIX=$prefix"
if ($LASTEXITCODE -ne 0) { throw 'SDK configuration failed. Use a Visual Studio developer shell with Clang 20+.' }
& cmake --build $build --target install -j $Jobs
if ($LASTEXITCODE -ne 0) { throw 'SDK build failed.' }
Write-Host "Patched SDK installed to $prefix"
