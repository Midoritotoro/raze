cmake_minimum_required(VERSION 4.1 FATAL_ERROR)

project(raze
    VERSION 1.0.0
    DESCRIPTION "RAZE"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CLANG_DEFAULT_RTLIB compiler-rt)

include(CMakePackageConfigHelpers)
include(CheckCXXCompilerFlag)

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR "lib")
endif()

if(NOT DEFINED CMAKE_INSTALL_BINDIR)
    set(CMAKE_INSTALL_BINDIR "bin")
endif()

if(NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
    set(CMAKE_INSTALL_INCLUDEDIR "include")
endif()


set(CMAKE_VERBOSE_MAKEFILE OFF)
include(cmake/tools/ClangRtBuiltins.cmake)

option(
    RAZE_INSTALL
    "Install RAZE"
    ${PROJECT_IS_TOP_LEVEL}
)

set(
    RAZE_INSTALL_CMAKE_DIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/raze"
)

set(RAZE_PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/raze/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/raze/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/raze/*.inl
)

file(
    GLOB_RECURSE RAZE_PUBLIC_HEADERS
    CONFIGURE_DEPENDS
    ${RAZE_PUBLIC_HEADERS}
)

add_library(raze INTERFACE)
add_library(raze::raze ALIAS raze)

target_compile_features(
    raze
    INTERFACE
    cxx_std_23
)

target_include_directories(
    raze
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(
    raze
    PROPERTIES
    EXPORT_NAME raze
)

if(MSVC)
    target_compile_options(
        raze
        INTERFACE
        /bigobj
        /permissive-
        /utf-8
        /Zc:__cplusplus
        /nologo
    )
endif()
raze_link_clang_rt_builtins(raze)

message(STATUS "RAZE compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "RAZE system processor: ${CMAKE_SYSTEM_PROCESSOR}")

if(CMAKE_GENERATOR MATCHES "Visual Studio"
    AND (
        CMAKE_VS_PLATFORM_NAME STREQUAL "x64"
        OR CMAKE_VS_PLATFORM_NAME STREQUAL "x86"
    )
)
    include(cmake/x86_Simd.cmake)
elseif(
    CMAKE_SYSTEM_PROCESSOR MATCHES
    "^(i[3-6]86|x86_64|AMD64|amd64)$"
)
    include(cmake/x86_Simd.cmake)
endif()

if(PROJECT_IS_TOP_LEVEL)
    enable_testing()

    add_subdirectory(tests)
    add_subdirectory(benchmarks)
endif()

if(RAZE_INSTALL AND NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/Util_InstallRules.cmake)
endif()
