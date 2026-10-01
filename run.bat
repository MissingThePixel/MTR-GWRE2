@echo off
setlocal
pushd "%~dp0"
if not exist "Game\default.xex" (
    echo Copy your complete extracted game into the Game folder first.
    echo The file Game\default.xex is required.
    pause
    popd
    exit /b 1
)
if not exist "logs" mkdir "logs"
"%~dp0gw2_recompiled.exe" --game_data_root "%~dp0Game" --user_data_root "%~dp0userdata" --cache_root "%~dp0cache" --execute_unclipped_draw_vs_on_cpu --gpu_plugin=xenos --license_mask=1 --d3d12_present_vsync --resolution=1080p --resolution_scale=2 --fullscreen --log_level=warn --log_verbose=false --guest_frame_stats=false --gpu_command_stats=false --gpu_slow_frame_stats=false --d3d12_fence_stats=false --log_file "%~dp0logs\geometry-wars-2-4k.log" %*
set "GAME_EXIT_CODE=%errorlevel%"
popd
if not "%GAME_EXIT_CODE%"=="0" pause
exit /b %GAME_EXIT_CODE%
