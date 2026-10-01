# gw2_recompiled - ReXGlue Recompiled Project
#
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(gw2_recompiled LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MTR_GAME_MANIFEST "${CMAKE_CURRENT_SOURCE_DIR}/gw2_recompiled_manifest.toml" CACHE FILEPATH "Local game manifest")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/generated/rexglue.cmake" MTR_SDK_INTEGRATION)
string(REPLACE "codegen \${CMAKE_CURRENT_SOURCE_DIR}/gw2_recompiled_manifest.toml"
    "codegen \"${MTR_GAME_MANIFEST}\"" MTR_SDK_INTEGRATION "${MTR_SDK_INTEGRATION}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/mtr-rexglue.cmake" "${MTR_SDK_INTEGRATION}")
include("${CMAKE_CURRENT_BINARY_DIR}/mtr-rexglue.cmake")

# Sources
set(GW2RECOMPILED_SOURCES
    src/main.cpp
    src/compat_math.cpp
)

if(WIN32)
    add_executable(gw2_recompiled WIN32 ${GW2RECOMPILED_SOURCES})
else()
    add_executable(gw2_recompiled ${GW2RECOMPILED_SOURCES})
endif()

rexglue_setup_target(gw2_recompiled)

if(MSVC)
    target_link_options(gw2_recompiled PRIVATE "/MAP:${CMAKE_CURRENT_BINARY_DIR}/gw2_recompiled.map")
endif()
