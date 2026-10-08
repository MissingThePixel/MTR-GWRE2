param(
    [Parameter(Mandatory=$true)][string]$GameDataRoot,
    [string]$AndroidSdk = $env:ANDROID_HOME,
    [string]$JavaHome = $env:JAVA_HOME,
    [string]$HostSdkPrefix = (Join-Path $PSScriptRoot '..\sdk-install-vulkan'),
    [string]$NdkVersion = '27.2.12479018',
    [int]$Jobs = 4
)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$android = Join-Path $root 'android'
$sdk = Join-Path $root 'thirdparty\rexglue-sdk'
if (!$AndroidSdk) { $AndroidSdk = Join-Path $env:LOCALAPPDATA 'Android\Sdk' }
if (!$JavaHome) {
    $localJdk = Get-ChildItem (Join-Path $android 'tools\jdk') -Directory -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($localJdk) { $JavaHome = $localJdk.FullName }
}
$AndroidSdk = (Resolve-Path -LiteralPath $AndroidSdk).Path
$ndk = Join-Path $AndroidSdk "ndk\$NdkVersion"
$toolchain = Join-Path $ndk 'build\cmake\android.toolchain.cmake'
$java = Join-Path $JavaHome 'bin\java.exe'
$codegen = Join-Path $HostSdkPrefix 'bin\rexglue.exe'
foreach ($required in @($toolchain, $java, $codegen, (Join-Path $GameDataRoot 'default.xex'))) {
    if (!(Test-Path -LiteralPath $required)) { throw "Missing build input: $required. See android/README.md." }
}
function Invoke-Checked([string]$Executable, [string[]]$Arguments) {
    & $Executable @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Executable failed with exit code $LASTEXITCODE" }
}
function Apply-Patch([string]$Name) {
    $patch = Join-Path $root "patches\$Name"
    & git -C $sdk apply --check --reverse $patch 2>$null
    if ($LASTEXITCODE -eq 0) { return }
    Invoke-Checked git @('-C',$sdk,'apply','--check',$patch)
    Invoke-Checked git @('-C',$sdk,'apply',$patch)
}
$head = (& git -C $sdk rev-parse HEAD).Trim()
if ($head -ne 'f5337cdc947ff6d4c4196737e2c807a48f2a1fc2') { throw 'Use the pinned SDK prepared by setup-sdk.ps1 -VulkanOnly.' }
Apply-Patch 'rexglue-vulkan-texture-exponent.patch'
Apply-Patch 'rexglue-vulkan-present-pipeline.patch'
Apply-Patch 'rexglue-android-arm64.patch'
$GameDataRoot = (Resolve-Path -LiteralPath $GameDataRoot).Path
$manifest = Join-Path $root 'source\gw2_local.toml'
$content = [IO.File]::ReadAllText((Join-Path $root 'source\gw2_recompiled_manifest.toml'))
$gamePath = $GameDataRoot.Replace('\','/').Replace('"','\"')
$xexPath = (Join-Path $GameDataRoot 'default.xex').Replace('\','/').Replace('"','\"')
$content = $content -replace 'game_root = "[^"]*"', ('game_root = "'+$gamePath+'"')
$content = $content -replace 'file_path = "[^"]*"', ('file_path = "'+$xexPath+'"')
[IO.File]::WriteAllText($manifest, $content, (New-Object Text.UTF8Encoding($false)))
Invoke-Checked $codegen @('codegen',$manifest)
$common = @('-G','Ninja',"-DCMAKE_TOOLCHAIN_FILE=$toolchain",'-DANDROID_ABI=arm64-v8a',
    '-DANDROID_PLATFORM=android-28','-DANDROID_STL=c++_shared','-DCMAKE_BUILD_TYPE=Release',
    '-DCMAKE_SHARED_LINKER_FLAGS=-Wl,-z,max-page-size=16384')
$sdkBuild = Join-Path $android 'out\sdk-arm64'
$gameBuild = Join-Path $android 'out\game-arm64'
Invoke-Checked cmake (@('-S',$sdk,'-B',$sdkBuild) + $common + @('-DREXGLUE_USE_VULKAN=ON',
    '-DREXGLUE_USE_D3D12=OFF','-DREXGLUE_ENABLE_TRACY=OFF','-DREXGLUE_ENABLE_PERF_COUNTERS=OFF'))
Invoke-Checked cmake @('--build',$sdkBuild,'--target','rexruntime','rexgpu-xenos','-j',"$Jobs")
Invoke-Checked cmake (@('-S',$android,'-B',$gameBuild) + $common)
Invoke-Checked cmake @('--build',$gameBuild,'-j',"$Jobs")
$stage = Join-Path $android 'app\src\main\jniLibs\arm64-v8a'
New-Item -ItemType Directory -Path $stage -Force | Out-Null
$sdkLib = Join-Path $sdk 'out\android-arm64'
foreach ($name in @('librexruntime.so','librexgpu-xenos.so','libSDL3.so')) {
    Copy-Item -LiteralPath (Join-Path $sdkLib $name) -Destination $stage -Force
}
Copy-Item -LiteralPath (Join-Path $gameBuild 'libmain.so') -Destination $stage -Force
$ndkHost = Join-Path $ndk 'toolchains\llvm\prebuilt\windows-x86_64'
Copy-Item -LiteralPath (Join-Path $ndkHost 'sysroot\usr\lib\aarch64-linux-android\libc++_shared.so') -Destination $stage -Force
$strip = Join-Path $ndkHost 'bin\llvm-strip.exe'
foreach ($library in Get-ChildItem -LiteralPath $stage -Filter '*.so') {
    Invoke-Checked $strip @('--strip-unneeded',$library.FullName)
}
[IO.File]::WriteAllText((Join-Path $android 'local.properties'), 'sdk.dir='+$AndroidSdk.Replace('\','/')+"`n")
$env:JAVA_HOME = $JavaHome
$env:GRADLE_USER_HOME = Join-Path $android 'tools\gradle-cache'
Invoke-Checked $java @('-classpath',(Join-Path $android 'gradle\wrapper\gradle-wrapper.jar'),
    'org.gradle.wrapper.GradleWrapperMain','-p',$android,'assembleRelease','--no-daemon','--console=plain')
Write-Host "APK: $(Join-Path $android 'app\build\outputs\apk\release\app-release.apk')"