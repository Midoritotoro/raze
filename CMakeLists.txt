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

option(
    RAZE_INSTALL
    "Install RAZE"
    ${PROJECT_IS_TOP_LEVEL}
)

option(
    RAZE_ENABLE_OPENMP
    "Enable OpenMP support in RAZE"
    OFF
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

set(RAZE_FORCE_LIST
    RAZE_FORCE_SSE2
    RAZE_FORCE_SSE3
    RAZE_FORCE_SSSE3
    RAZE_FORCE_SSE41
    RAZE_FORCE_SSE42
    RAZE_FORCE_AVX
    RAZE_FORCE_FMA3
    RAZE_FORCE_AVX2
    RAZE_FORCE_AVX512F
    RAZE_FORCE_AVX512BW
    RAZE_FORCE_AVX512CD
    RAZE_FORCE_AVX512DQ
    RAZE_FORCE_AVX512VL
    RAZE_FORCE_AVX512VBMI
    RAZE_FORCE_AVX512VBMI2
)

foreach(force_flag IN LISTS RAZE_FORCE_LIST)
    option(${force_flag} "Force ${force_flag}" OFF)
endforeach()

set(RAZE_USER_FORCE_ENABLED OFF)

foreach(force_flag IN LISTS RAZE_FORCE_LIST)
    if(${force_flag})
        set(RAZE_USER_FORCE_ENABLED ON)
    endif()
endforeach()

function(raze_require_compile_flag force_flag compile_flag)
    if(${force_flag})
        string(MAKE_C_IDENTIFIER
            "${compile_flag}"
            compile_flag_id
        )

        set(check_variable
            "RAZE_${compile_flag_id}_SUPPORTED"
        )

        check_cxx_compiler_flag(
            "${compile_flag}"
            ${check_variable}
        )

        if(NOT ${check_variable})
            message(FATAL_ERROR
                "${force_flag}=ON requires compiler flag "
                "${compile_flag}, but it is not supported"
            )
        endif()

        target_compile_options(
            raze
            INTERFACE
            "${compile_flag}"
        )

        target_compile_definitions(
            raze
            INTERFACE
            "${force_flag}=1"
        )
    endif()
endfunction()

if(RAZE_USER_FORCE_ENABLED)
    message(STATUS "RAZE: ISA FORCE mode enabled")

    if(MSVC)
        if(RAZE_FORCE_AVX)
            raze_require_compile_flag(
                RAZE_FORCE_AVX
                "/arch:AVX"
            )
        endif()

        if(RAZE_FORCE_AVX2 OR RAZE_FORCE_FMA3)
            check_cxx_compiler_flag(
                "/arch:AVX2"
                RAZE_MSVC_AVX2_SUPPORTED
            )

            if(NOT RAZE_MSVC_AVX2_SUPPORTED)
                message(FATAL_ERROR
                    "RAZE_FORCE_AVX2/RAZE_FORCE_FMA3 requires "
                    "MSVC /arch:AVX2 support"
                )
            endif()

            target_compile_options(
                raze
                INTERFACE
                /arch:AVX2
            )
        endif()

        if(RAZE_FORCE_AVX2)
            target_compile_definitions(
                raze
                INTERFACE
                RAZE_FORCE_AVX2=1
            )
        endif()

        if(RAZE_FORCE_FMA3)
            target_compile_definitions(
                raze
                INTERFACE
                RAZE_FORCE_FMA3=1
            )
        endif()

        if(
            RAZE_FORCE_AVX512F
            OR RAZE_FORCE_AVX512BW
            OR RAZE_FORCE_AVX512CD
            OR RAZE_FORCE_AVX512DQ
            OR RAZE_FORCE_AVX512VL
            OR RAZE_FORCE_AVX512VBMI
            OR RAZE_FORCE_AVX512VBMI2
        )
            check_cxx_compiler_flag(
                "/arch:AVX512"
                RAZE_MSVC_AVX512_SUPPORTED
            )

            if(NOT RAZE_MSVC_AVX512_SUPPORTED)
                message(FATAL_ERROR
                    "AVX-512 was requested, but MSVC does not support "
                    "/arch:AVX512"
                )
            endif()

            target_compile_options(
                raze
                INTERFACE
                /arch:AVX512
            )
        endif()

        foreach(force_flag IN LISTS RAZE_FORCE_LIST)
            if(${force_flag})
                target_compile_definitions(
                    raze
                    INTERFACE
                    "${force_flag}=1"
                )
            endif()
        endforeach()

    else()
        raze_require_compile_flag(
            RAZE_FORCE_SSE2
            "-msse2"
        )

        raze_require_compile_flag(
            RAZE_FORCE_SSE3
            "-msse3"
        )

        raze_require_compile_flag(
            RAZE_FORCE_SSSE3
            "-mssse3"
        )

        raze_require_compile_flag(
            RAZE_FORCE_SSE41
            "-msse4.1"
        )

        raze_require_compile_flag(
            RAZE_FORCE_SSE42
            "-msse4.2"
        )

        raze_require_compile_flag(
            RAZE_FORCE_AVX
            "-mavx"
        )

        raze_require_compile_flag(
            RAZE_FORCE_FMA3
            "-mfma"
        )

        raze_require_compile_flag(
            RAZE_FORCE_AVX2
            "-mavx2"
        )

        raze_require_compile_flag(
            RAZE_FORCE_AVX512F
            "-mavx512f"
        )

        raze_require_compile_flag(
            RAZE_FORCE_AVX512BW
            "-mavx512bw"
        )

        raze_require_compile_flag(
            RAZE_FORCE_AVX512CD
            "-mavx512cd"
        )

        raze_require_compile_flag(
            RAZE_FORCE_AVX512DQ
            "-mavx512dq"
        )

        raze_require_compile_flag(
            RAZE_FORCE_AVX512VL
            "-mavx512vl"
        )

        raze_require_compile_flag(
            RAZE_FORCE_AVX512VBMI
            "-mavx512vbmi"
        )

        raze_require_compile_flag(
            RAZE_FORCE_AVX512VBMI2
            "-mavx512vbmi2"
        )
    endif()

    target_compile_definitions(
        raze
        INTERFACE
        RAZE_ISA_FORCE_ENABLED=1
    )
else()
    message(STATUS
        "RAZE: no FORCE ISA specified — using runtime dispatch"
    )

    target_compile_definitions(
        raze
        INTERFACE
        RAZE_ISA_FORCE_ENABLED=0
    )
endif()

if(RAZE_ENABLE_OPENMP)
    find_package(OpenMP REQUIRED COMPONENTS CXX)

    target_link_libraries(
        raze
        INTERFACE
        OpenMP::OpenMP_CXX
    )

    target_compile_definitions(
        raze
        INTERFACE
        RAZE_ENABLE_OPENMP=1
    )

    message(STATUS "RAZE: OpenMP enabled")
else()
    target_compile_definitions(
        raze
        INTERFACE
        RAZE_ENABLE_OPENMP=0
    )
endif()

if(PROJECT_IS_TOP_LEVEL)
    enable_testing()

    add_subdirectory(tests)
    add_subdirectory(benchmarks)
endif()

if(RAZE_INSTALL AND NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/Util_InstallRules.cmake)
endif()
