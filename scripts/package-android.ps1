param(
    [string]$Apk = (Join-Path $PSScriptRoot '../android/app/build/outputs/apk/release/app-release.apk'),
    [string]$OutputDirectory = (Join-Path $PSScriptRoot '../dist/android-v0.1.0')
)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
$package = Join-Path $OutputDirectory 'MTR-GWRE2-android-arm64'
$zip = Join-Path $OutputDirectory 'MTR-GWRE2-android-arm64.zip'
$standalone = Join-Path $OutputDirectory 'MTR-GWRE2-android-arm64.apk'
foreach ($path in @($package,$zip,$standalone)) {
    if (Test-Path -LiteralPath $path) { throw 'Select an empty output directory; existing packages are not overwritten.' }
}
if (!(Test-Path -LiteralPath $Apk)) { throw "Missing release APK: $Apk" }
New-Item -ItemType Directory -Path (Join-Path $package 'LICENSES') -Force | Out-Null
Copy-Item -LiteralPath $Apk -Destination $standalone
Copy-Item -LiteralPath $Apk -Destination (Join-Path $package 'MTR-GWRE2-android-arm64.apk')
$readme = [IO.File]::ReadAllText((Join-Path $root 'android/README.md'))
$readme = $readme.Replace('(BUILDING.md)','(https://github.com/MissingThePixel/MTR-GWRE2/blob/main/android/BUILDING.md)').Replace('(../THIRD_PARTY.md)','(THIRD_PARTY.md)').Replace('(../LICENSE)','(LICENSE.txt)')
[IO.File]::WriteAllText((Join-Path $package 'README.md'),$readme,(New-Object Text.UTF8Encoding($false)))
Copy-Item -LiteralPath (Join-Path $root 'LICENSE') -Destination (Join-Path $package 'LICENSE.txt')
Copy-Item -LiteralPath (Join-Path $root 'THIRD_PARTY.md') -Destination $package
$allowed = @('MTR-GWRE2-android-arm64.apk','README.md','LICENSE.txt','THIRD_PARTY.md')
foreach ($file in Get-ChildItem -LiteralPath (Join-Path $root 'LICENSES') -Filter '*.txt' -File) {
    Copy-Item -LiteralPath $file.FullName -Destination (Join-Path $package 'LICENSES')
    $allowed += 'LICENSES' + [IO.Path]::DirectorySeparatorChar + $file.Name
}
foreach ($file in Get-ChildItem -LiteralPath $package -Recurse -File) {
    $relative = $file.FullName.Substring($package.Length+1)
    if ($relative -notin $allowed) { throw "Unexpected release file: $relative" }
}
Compress-Archive -LiteralPath $package -DestinationPath $zip -CompressionLevel Optimal
$checksums = foreach ($file in @($standalone,$zip)) {
    $hash = Get-FileHash -LiteralPath $file -Algorithm SHA256
    $hash.Hash.ToLowerInvariant() + '  ' + [IO.Path]::GetFileName($file)
}
[IO.File]::WriteAllLines((Join-Path $OutputDirectory 'SHA256SUMS.txt'),$checksums,(New-Object Text.UTF8Encoding($false)))
Write-Host "Android package: $zip"
