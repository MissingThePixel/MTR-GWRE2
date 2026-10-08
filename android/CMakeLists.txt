cmake_minimum_required(VERSION 3.25)
project(MTRGWRE2Android LANGUAGES C CXX)
if(NOT ANDROID OR NOT ANDROID_ABI STREQUAL "arm64-v8a")
    message(FATAL_ERROR "This target requires the Android NDK arm64-v8a toolchain")
endif()
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
get_filename_component(MTR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(MTR_SDK "${MTR_ROOT}/thirdparty/rexglue-sdk")
set(MTR_SDK_BUILD "${CMAKE_CURRENT_SOURCE_DIR}/out/sdk-arm64")
set(MTR_SDK_LIB "${MTR_SDK}/out/android-arm64")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include("${MTR_ROOT}/source/generated/default/sources.cmake")
add_library(rexruntime SHARED IMPORTED)
set_target_properties(rexruntime PROPERTIES IMPORTED_LOCATION "${MTR_SDK_LIB}/librexruntime.so")
add_library(SDL3 SHARED IMPORTED)
set_target_properties(SDL3 PROPERTIES IMPORTED_LOCATION "${MTR_SDK_LIB}/libSDL3.so")
add_library(main SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/gpu_poll_backoff.cpp"
    "${MTR_ROOT}/source/src/main.cpp"
    "${MTR_ROOT}/source/src/compat_math.cpp"
    "${MTR_SDK}/src/ui/windowed_app_main_sdl.cpp"
    "${MTR_SDK}/src/ui/rex_app.cpp"
    ${GENERATED_SOURCES})
target_include_directories(main PRIVATE
    "${MTR_ROOT}/source" "${MTR_ROOT}/source/src"
    "${MTR_ROOT}/source/generated/default"
    "${MTR_SDK}/include" "${MTR_SDK_BUILD}/include"
    "${MTR_SDK}/thirdparty/fmt/include" "${MTR_SDK}/thirdparty/spdlog/include"
    "${MTR_SDK}/thirdparty/simde" "${MTR_SDK}/thirdparty/simde/simde"
    "${MTR_SDK}/thirdparty/tomlplusplus/include"
    "${MTR_SDK}/thirdparty/sdl3/include" "${MTR_SDK}/thirdparty/imgui")
target_compile_definitions(main PRIVATE REX_HAS_VULKAN=1 REXGLUE_BUILD_CONFIG="Release" SPDLOG_FMT_EXTERNAL=1)
target_compile_options(main PRIVATE -fexperimental-library -fno-char8_t -fno-strict-aliasing -ffp-model=strict -Wno-everything)
target_link_options(main PRIVATE -fexperimental-library -Wl,-z,max-page-size=16384)
target_link_libraries(main PRIVATE rexruntime SDL3 android log dl)
