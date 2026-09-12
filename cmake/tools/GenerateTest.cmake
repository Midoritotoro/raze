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
    "SSE2|-msse2| |RAZE_HAS_SSE2_SUPPORT=1"
    "SSE3|-msse3| |RAZE_HAS_SSE3_SUPPORT=1"
    "SSSE3|-mssse3| |RAZE_HAS_SSSE3_SUPPORT=1"
    "SSE41|-msse4.1| |RAZE_HAS_SSE41_SUPPORT=1"
    "SSE42|-msse4.2| |RAZE_HAS_SSE42_SUPPORT=1"
    "AVX|-mavx|/arch:AVX|RAZE_HAS_AVX_SUPPORT=1"
    "FMA3|-mfma|/arch:AVX|RAZE_HAS_FMA3_SUPPORT=1"
    "AVX2|-mavx2|/arch:AVX2|RAZE_HAS_AVX2_SUPPORT=1"
    "AVX512F|-mavx512f|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1"
    "AVX512BW|-mavx512bw|/arch:AVX2|RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512F_SUPPORT=1"
    "AVX512DQ|-mavx512dq|/arch:AVX2|RAZE_HAS_AVX512DQ_SUPPORT=1,RAZE_HAS_AVX512F_SUPPORT=1"
    "AVX512BWDQ|-mavx512bw -mavx512dq|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1"
    "AVX512VLBWDQ|-mavx512vl -mavx512bw -mavx512dq|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1"
    "AVX512VLDQ|-mavx512vl -mavx512dq|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1"
    "AVX512VLBW|-mavx512vl -mavx512bw|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1"
    "AVX512VLF|-mavx512f -mavx512vl|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1"
    "AVX512VBMI|-mavx512vbmi -mavx512bw|/arch:AVX2|RAZE_HAS_AVX512VBMI_SUPPORT=1,RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1"
    "AVX512VBMI2|-mavx512vbmi2 -mavx512bw|/arch:AVX2|RAZE_HAS_AVX512VBMI2_SUPPORT=1,RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512F_SUPPORT=1"
    "AVX512VBMIVL|-mavx512vbmi -mavx512vl|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1"
    "AVX512VBMI2VL|-mavx512vbmi2 -mavx512vl|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1"
    "AVX512VBMIDQ|-mavx512vbmi -mavx512bw -mavx512dq|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1"
    "AVX512VBMI2DQ|-mavx512vbmi2 -mavx512bw -mavx512dq|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1"
    "AVX512VBMIVLDQ|-mavx512vbmi -mavx512bw -mavx512dq -mavx512vl|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1"
    "AVX512VBMI2VLDQ|-mavx512vbmi2 -mavx512bw -mavx512dq -mavx512vl|/arch:AVX2|RAZE_HAS_AVX512F_SUPPORT=1,RAZE_HAS_AVX512BW_SUPPORT=1,RAZE_HAS_AVX512DQ_SUPPORT=1,RAZE_HAS_AVX512VL_SUPPORT=1"
)

function(raze_generate_test root main_source rootpath file)
    string(REPLACE ".cpp" "" base ${file})
    string(REPLACE "/" "." base ${base})
    string(REPLACE "\\" "." base ${base})

    if(NOT root STREQUAL "")
        set(test "${root}.${base}")
    else()
        set(test "${base}")
    endif()

    if(NOT EXISTS "${main_source}")
        message(FATAL_ERROR "raze_generate_test: main.cpp не найден: ${main_source}")
    endif()

    foreach(arch_config ${RAZE_TEST_ARCH_CONFIGS})
        string(REPLACE "|" ";" config_list "${arch_config}")
        list(GET config_list 0 arch_name)
        list(GET config_list 1 gcc_flags)
        list(GET config_list 2 msvc_arch_flag)
        list(GET config_list 3 msvc_defs)

        set(arch_test "${test}_${arch_name}")
        add_executable(${arch_test} "${main_source}" "${rootpath}${file}")

        set_target_properties(${arch_test} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/unit"
            EXCLUDE_FROM_DEFAULT_BUILD TRUE
            EXCLUDE_FROM_ALL TRUE
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            CXX_SCAN_FOR_MODULES OFF
        )

        target_compile_features(${arch_test} PUBLIC cxx_std_23)
        target_link_libraries(${arch_test} PRIVATE raze::raze)
        target_include_directories(${arch_test} PRIVATE
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/include
        )

        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            string(REPLACE "," ";" msvc_defs_list "${msvc_defs}")
            target_compile_definitions(${arch_test} PRIVATE ${msvc_defs_list})
            target_compile_options(${arch_test} PRIVATE
                /bigobj /permissive- /Od /MP ${msvc_arch_flag}
            )
        else()
            separate_arguments(flags_list UNIX_COMMAND "${gcc_flags}")
            target_compile_options(${arch_test} PRIVATE ${flags_list} -g)
        endif()

        add_test(
            NAME ${arch_test}
            WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/unit"
            COMMAND $<TARGET_FILE:${arch_test}>
        )

        add_dependencies(unit ${arch_test})
    endforeach()

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