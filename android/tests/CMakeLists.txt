cmake_minimum_required(VERSION 3.25)
project(MTRAndroidNativeTests LANGUAGES CXX ASM)
set(CMAKE_CXX_STANDARD 23)
get_filename_component(SDK "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/rexglue-sdk" ABSOLUTE)
add_executable(fiber-smoke fiber-smoke.cpp "${SDK}/src/core/fiber_android.cpp" "${SDK}/src/core/fiber_android_arm64.S")
target_include_directories(fiber-smoke PRIVATE "${SDK}/include")


add_executable(write-watch-smoke write-watch-smoke.cpp)
target_include_directories(write-watch-smoke PRIVATE "${SDK}/include")
target_link_libraries(write-watch-smoke PRIVATE "${SDK}/out/android-arm64/librexruntime.so")

add_executable(physical-watch-smoke physical-watch-smoke.cpp)
target_include_directories(physical-watch-smoke PRIVATE "${SDK}/include")
target_link_libraries(physical-watch-smoke PRIVATE "${SDK}/out/android-arm64/librexruntime.so")

add_executable(shared-memory-watch-smoke shared-memory-watch-smoke.cpp "${SDK}/src/graphics/shared_memory.cpp")
target_include_directories(shared-memory-watch-smoke PRIVATE "${SDK}/include" "${SDK}/thirdparty/fmt/include" "${SDK}/thirdparty/spdlog/include")
target_compile_definitions(shared-memory-watch-smoke PRIVATE SPDLOG_FMT_EXTERNAL=1)
target_link_libraries(shared-memory-watch-smoke PRIVATE "${SDK}/out/android-arm64/librexruntime.so")
