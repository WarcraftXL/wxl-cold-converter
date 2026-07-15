# wxl-converter: a DLL that pre-bakes on disk what wxl-core's modules otherwise reshape in memory
# at serve time. Independent build (no dependency on wxl-core); shared byte-transform files are
# copied in, not linked, so this tree never needs the wxl-core CMake/toolchain context.
# Ships as a native DLL (no CLI/exe form) with a C ABI (src/api/Api.hpp) meant to sit next to and
# be P/Invoked by wxl-converter-ui (the WPF front end) -- see that project for the other half.
# Copyright (C) 2026 WarcraftXL. GPLv3 (see source headers).

cmake_minimum_required(VERSION 3.20)
project(wxl-converter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE WXL_CONVERTER_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(wxl-converter SHARED ${WXL_CONVERTER_SRC})
target_include_directories(wxl-converter PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_definitions(wxl-converter PRIVATE
    WIN32_LEAN_AND_MEAN NOMINMAX _CRT_SECURE_NO_WARNINGS
    WXL_HOST # the copied shared/ theme files gate their live-engine-only half on this (see Particles/Ribbons)
)

if(MSVC)
    target_compile_options(wxl-converter PRIVATE /W4)
endif()
