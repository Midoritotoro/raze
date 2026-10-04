cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(razeBenchmarks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE ON)

if(PROJECT_IS_TOP_LEVEL)
    find_package(raze REQUIRED)
endif()

enable_testing()

# ------------------------------------------------------------------------------
# Google Benchmark lives in raze/benchmarks/benchmark
# ------------------------------------------------------------------------------

set(BENCHMARK_ENABLE_DOXYGEN OFF)
set(BENCHMARK_ENABLE_INSTALL OFF)
set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF)

add_subdirectory(benchmark EXCLUDE_FROM_ALL)

option(RAZE_BENCHMARKS_COMMON_OPTIMIZATION
    "Add non-ISA optimization flags such as /O2 or -O2 to benchmark targets"
    ON
)

option(RAZE_BENCHMARKS_NATIVE_MARCH_NATIVE
    "Add -march=native to native benchmark targets when compiler supports it"
    ON
)

option(RAZE_BENCHMARKS_REGISTER_TESTS
    "Register benchmark executables as CTest tests"
    ON
)

include(CheckCXXCompilerFlag)

if(NOT DEFINED RAZE_CXX_SUPPORTS_MARCH_NATIVE)
    check_cxx_compiler_flag("-march=native" RAZE_CXX_SUPPORTS_MARCH_NATIVE)
endif()

function(_raze_bench_is_msvc_like out_var)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
        CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
        set(${out_var} TRUE PARENT_SCOPE)
    else()
        set(${out_var} FALSE PARENT_SCOPE)
    endif()
endfunction()

function(_raze_bench_detect_source_root out_var)
    set(_candidates)

    if(DEFINED RAZE_SOURCE_DIR AND NOT "${RAZE_SOURCE_DIR}" STREQUAL "")
        list(APPEND _candidates "${RAZE_SOURCE_DIR}")
    endif()

    if(NOT PROJECT_IS_TOP_LEVEL)
        list(APPEND _candidates "${CMAKE_SOURCE_DIR}")
    endif()
    list(APPEND _candidates
        "${CMAKE_CURRENT_SOURCE_DIR}/../.."
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    foreach(_candidate IN LISTS _candidates)
        get_filename_component(_abs "${_candidate}" ABSOLUTE)

        if(EXISTS "${_abs}/src/raze" OR
           EXISTS "${_abs}/include/raze" OR
           EXISTS "${_abs}/raze")
            set(${out_var} "${_abs}" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    get_filename_component(_abs "${CMAKE_SOURCE_DIR}" ABSOLUTE)
    set(${out_var} "${_abs}" PARENT_SCOPE)
endfunction()

set(RAZE_BENCH_SOURCE_ROOT "" CACHE PATH
    "Path to RAZE source root containing src/raze and/or include/raze"
)

if("${RAZE_BENCH_SOURCE_ROOT}" STREQUAL "")
    _raze_bench_detect_source_root(_raze_detected_root)
    set(RAZE_BENCH_SOURCE_ROOT "${_raze_detected_root}" CACHE PATH
        "Path to RAZE source root containing src/raze and/or include/raze"
        FORCE
    )
endif()

get_filename_component(RAZE_BENCH_SOURCE_ROOT "${RAZE_BENCH_SOURCE_ROOT}" ABSOLUTE)

message(STATUS "RAZE benchmark source root: ${RAZE_BENCH_SOURCE_ROOT}")

function(_raze_bench_prepare_target target source)
    set(source_path "${source}")

    if(NOT IS_ABSOLUTE "${source_path}")
        set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/${source_path}")
    endif()

    if(NOT EXISTS "${source_path}")
        message(FATAL_ERROR
            "raze benchmark source not found: ${source}\n"
            "Resolved as: ${source_path}"
        )
    endif()

    add_executable(${target} "${source_path}")

    target_compile_features(${target} PRIVATE cxx_std_23)
    set(_include_dirs "${CMAKE_CURRENT_SOURCE_DIR}/tools")

    foreach(_dir IN ITEMS
        "${RAZE_BENCH_SOURCE_ROOT}"
        "${RAZE_BENCH_SOURCE_ROOT}/include"
        "${RAZE_BENCH_SOURCE_ROOT}/src"
    )
        if(NOT "${_dir}" STREQUAL "" AND EXISTS "${_dir}")
            list(APPEND _include_dirs "${_dir}")
        endif()
    endforeach()

    list(REMOVE_DUPLICATES _include_dirs)

    target_include_directories(${target} PRIVATE
        ${_include_dirs}
    )

    target_link_libraries(${target} PRIVATE
        raze::raze
        benchmark::benchmark_main
    )

    if(RAZE_BENCHMARKS_COMMON_OPTIMIZATION)
        _raze_bench_is_msvc_like(msvc_like)

        if(msvc_like)
            target_compile_options(${target} PRIVATE
                /O2
                /Ob3
                /bigobj
                /GS-
                /Oy
            )

            if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                target_compile_options(${target} PRIVATE
                    /permissive-
                )
            endif()

            target_compile_definitions(${target} PRIVATE
                NDEBUG
            )
        else()
            target_compile_options(${target} PRIVATE
                -O2
                -DNDEBUG
            )
        endif()
    endif()

    set_target_properties(${target} PROPERTIES
        FOLDER "Benchmarks"
    )
endfunction()

add_custom_target(benchmark_native)
add_custom_target(benchmark_portable)

add_custom_target(benchmarks)
add_dependencies(benchmarks
    benchmark_native
    benchmark_portable
)

function(generate_benchmark_native filename)
    set(target "${filename}.native")
    set(source "${filename}.cpp")

    _raze_bench_prepare_target(${target} "${source}")

    if(RAZE_BENCHMARKS_NATIVE_MARCH_NATIVE AND RAZE_CXX_SUPPORTS_MARCH_NATIVE)
        target_compile_options(${target} PRIVATE
            -march=native
        )
    endif()

    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME "${filename}.native"
        FOLDER "Benchmarks/Native"
    )

    if(RAZE_BENCHMARKS_REGISTER_TESTS)
        add_test(
            NAME ${target}
            COMMAND ${target}
        )

        set_tests_properties(${target} PROPERTIES
            LABELS "benchmark;benchmark.native"
        )
    endif()

    add_dependencies(benchmark_native ${target})
endfunction()

function(generate_benchmark_portable filename)
    set(target "${filename}.portable")
    set(source "${filename}.cpp")

    _raze_bench_prepare_target(${target} "${source}")

    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME "${filename}.portable"
        FOLDER "Benchmarks/Portable"
    )

    if(RAZE_BENCHMARKS_REGISTER_TESTS)
        add_test(
            NAME ${target}
            COMMAND ${target}
        )

        set_tests_properties(${target} PROPERTIES
            LABELS "benchmark;benchmark.portable"
        )
    endif()

    add_dependencies(benchmark_portable ${target})
endfunction()

macro(razeBenchmarks_add_benchmark filename)
    generate_benchmark_native(${filename})
    generate_benchmark_portable(${filename})
endmacro()

file(GLOB BENCH_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

foreach(benchsource IN LISTS BENCH_SOURCES)
    get_filename_component(filename "${benchsource}" NAME_WE)

    if(NOT "${filename}" STREQUAL "main")
        razeBenchmarks_add_benchmark(${filename})
    endif()
endforeach()