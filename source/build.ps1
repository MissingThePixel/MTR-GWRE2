param([Parameter(Mandatory=$true)][string]$GameDataRoot,
      [string]$SdkPrefix = (Join-Path $PSScriptRoot '..\sdk-install'), [int]$Jobs = 4)
& (Join-Path $PSScriptRoot '..\scripts\build.ps1') -GameDataRoot $GameDataRoot -SdkPrefix $SdkPrefix -Jobs $Jobs
