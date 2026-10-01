@echo off
setlocal
pushd "%~dp0"
echo Geometry Wars: Retro Evolved 2
echo.
echo 1. 4K fullscreen (2x native rendering)
echo 2. Native 1080p fullscreen
echo.
choice /c 12 /n /m "Choose 1 or 2: "
if errorlevel 2 (call "%~dp0run-native.bat" %*) else (call "%~dp0run.bat" %*)
popd
