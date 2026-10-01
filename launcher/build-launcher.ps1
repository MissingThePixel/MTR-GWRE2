param([string]$OutputDirectory = (Join-Path $PSScriptRoot '..\source\out\launcher'))
$ErrorActionPreference = 'Stop'
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
Add-Type -AssemblyName System.Drawing
$iconPath = Join-Path $OutputDirectory 'launcher.ico'
$sourceImage = [Drawing.Image]::FromFile((Join-Path $PSScriptRoot 'launchericon.png'))
$stream = New-Object IO.MemoryStream
try {
    $bitmap = New-Object Drawing.Bitmap 256,256
    $graphics = [Drawing.Graphics]::FromImage($bitmap)
    try {
        $graphics.InterpolationMode = [Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
        $graphics.DrawImage($sourceImage, 0, 0, 256, 256)
        $bitmap.Save($stream, [Drawing.Imaging.ImageFormat]::Png)
    } finally { $graphics.Dispose(); $bitmap.Dispose() }
    $png = $stream.ToArray()
    $file = [IO.File]::Create($iconPath)
    $writer = New-Object IO.BinaryWriter $file
    try {
        $writer.Write([uint16]0); $writer.Write([uint16]1); $writer.Write([uint16]1)
        $writer.Write([byte]0); $writer.Write([byte]0); $writer.Write([byte]0); $writer.Write([byte]0)
        $writer.Write([uint16]1); $writer.Write([uint16]32); $writer.Write([uint32]$png.Length); $writer.Write([uint32]22)
        $writer.Write($png)
    } finally { $writer.Dispose() }
} finally { $sourceImage.Dispose(); $stream.Dispose() }
$compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
$output = Join-Path $OutputDirectory 'MTR-GWRE2.exe'
& $compiler /nologo /target:winexe /platform:x64 /optimize+ ('/out:' + $output) ('/win32icon:' + $iconPath) ('/resource:' + (Join-Path $PSScriptRoot 'launchericon.png') + ',launchericon.png') /reference:System.Windows.Forms.dll /reference:System.Drawing.dll /reference:System.Web.Extensions.dll (Join-Path $PSScriptRoot 'Launcher.cs')
if ($LASTEXITCODE -ne 0) { throw 'Launcher compilation failed.' }
Write-Host "Launcher built: $output"
