function(raze_add_parent_target target)
  string(REGEX REPLACE "[^.]+\\.([^.]+)$" "\\1" parent_target ${target})
  string(REGEX REPLACE "^.*\\.([^.]+)$" "\\1" suffix ${parent_target})

  if(NOT TARGET ${target})
    add_custom_target(${target})
    set_property(TARGET ${target} PROPERTY FOLDER "${suffix}")
  endif()

  if(NOT ${parent_target} STREQUAL ${target})
    raze_add_parent_target(${parent_target})
    add_dependencies(${parent_target} ${target})
  endif()
endfunction()

set(RAZE_TEST_ARCH_CONFIGS
    "SSE2|-msse2|RAZE_HAS_SSE2_SUPPORT=1"
    #"SSE3|-msse3|RAZE_HAS_SSE3_SUPPORT=1"
    #"SSSE3|-mssse3|RAZE_HAS_SSSE3_SUPPORT=1"
    #"SSE41|-msse4.1|RAZE_HAS_SSE41_SUPPORT=1"
    #"SSE42|-msse4.2|RAZE_HAS_SSE42_SUPPORT=1"
    #"AVX|-mavx|RAZE_HAS_AVX_SUPPORT=1"
    #"FMA3|-mfma -mavx|RAZE_HAS_FMA3_SUPPORT=1"
    #"AVX2|-mavx2 -mfma|RAZE_HAS_AVX2_SUPPORT=1"
    #"AVX512F|-mavx512f|RAZE_HAS_AVX512F_SUPPORT=1"
    #"AVX512BW|-mavx512bw -mavx512f|RAZE_HAS_AVX512BW_SUPPORT=1"
    #"AVX512DQ|-mavx512dq -mavx512f|RAZE_HAS_AVX512DQ_SUPPORT=1"
    #"AVX512VL|-mavx512vl -mavx512f|RAZE_HAS_AVX512VL_SUPPORT=1"
    #"AVX512VBMI|-mavx512vbmi -mavx512f|RAZE_HAS_AVX512VBMI_SUPPORT=1"
    #"AVX512VBMI2|-mavx512vbmi2 -mavx512f|RAZE_HAS_AVX512VBMI2_SUPPORT=1"
)

function(raze_generate_test root main_source rootpath file)
    string(REPLACE ".cpp" ".exe" base ${file})
    string(REPLACE "/" "." base ${base})
    string(REPLACE "\\" "." base ${base})

    if(NOT root STREQUAL "")
        set(test "${root}.${base}")
    else()
        set(test "${base}")
    endif()

    if(NOT EXISTS "${main_source}")
        message(FATAL_ERROR "raze_generate_test: main.cpp не найден по пути: ${main_source}")
    endif()

    add_executable(${test} "${main_source}")

    set_target_properties(${test} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/unit"
        EXCLUDE_FROM_DEFAULT_BUILD TRUE
        EXCLUDE_FROM_ALL TRUE
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        CXX_SCAN_FOR_MODULES OFF
    )

    target_compile_features(${test} PUBLIC cxx_std_23)
    target_link_libraries(${test} PRIVATE raze::raze)

    target_include_directories(${test} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/tests
    )

    foreach(arch_config ${RAZE_TEST_ARCH_CONFIGS})
        string(REPLACE "|" ";" config_list ${arch_config})
        list(GET config_list 0 arch_name)
        list(GET config_list 1 gcc_flags)
        list(GET config_list 2 msvc_defs)

        set(obj_target "${test}_${arch_name}")

        add_library(${obj_target} OBJECT "${rootpath}${file}")

        set_target_properties(${obj_target} PROPERTIES
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            CXX_SCAN_FOR_MODULES OFF
            EXCLUDE_FROM_DEFAULT_BUILD TRUE
            EXCLUDE_FROM_ALL TRUE
        )

        target_link_libraries(${obj_target} PRIVATE raze::raze)
        target_include_directories(${obj_target} PRIVATE
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/tests
        )

        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            target_compile_definitions(${obj_target} PRIVATE ${msvc_defs})
            target_compile_options(${obj_target} PRIVATE /bigobj /permissive- /O0 /Od)
        else()
            separate_arguments(flags_list UNIX_COMMAND "${gcc_flags}")
            target_compile_options(${obj_target} PRIVATE ${flags_list} -g -O0)
        endif()

        target_sources(${test} PRIVATE $<TARGET_OBJECTS:${obj_target}>)
    endforeach()

    if(RAZE_USE_PCH)
        target_precompile_headers(${test} PRIVATE
            <raze/test/rtts.hpp>
            <raze/vx/Algorithm.h>
            <raze/vx/Simd.h>
            <vector>
            <string>
            <iostream>
            <cmath>
            <random>
        )
    endif()

    add_test(
        NAME ${test}
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/unit"
        COMMAND $<TARGET_FILE:${test}>
    )

    add_dependencies(unit.exe ${test})
    raze_add_parent_target(${test})
endfunction()

function(raze_glob_unit root relative pattern)
    set(main_source "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    
    file(GLOB files RELATIVE ${relative} ${pattern})
    foreach(file ${files})
        raze_generate_test("${root}" "${main_source}" "${relative}/" ${file})
    endforeach()
endfunction()

function(raze_make_unit root)
    set(main_source "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    foreach(file ${ARGN})
        raze_generate_test(${root} "${main_source}" "${CMAKE_CURRENT_SOURCE_DIR}/" ${file})
    endforeach()
endfunction()