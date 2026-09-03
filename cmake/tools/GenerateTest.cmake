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
    "SSE2| | |RAZE_HAS_SSE2_SUPPORT=1"
    "AVX|-mavx|/arch:AVX|RAZE_HAS_AVX_SUPPORT=1"
    "AVX2|-mavx2|/arch:AVX2|RAZE_HAS_AVX2_SUPPORT=1"
    "AVX512|-mavx512f|/arch:AVX512|RAZE_HAS_AVX512F_SUPPORT=1"
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
        list(GET config_list 2 msvc_arch_flag)
        list(GET config_list 3 msvc_defs)

        set(obj_target "${test}_${arch_name}")

        add_library(${obj_target} OBJECT "${rootpath}${file}")

        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            target_compile_definitions(${obj_target} PRIVATE ${msvc_defs})
            
            set(msvc_defs_flags "")
            foreach(def ${msvc_defs})
                list(APPEND msvc_defs_flags "/D${def}")
            endforeach()
            
            target_compile_options(${obj_target} PRIVATE /bigobj /permissive- /O1 ${msvc_arch_flag} ${msvc_defs_flags})
        else()
            separate_arguments(flags_list UNIX_COMMAND "${gcc_flags}")
            target_compile_options(${obj_target} PRIVATE ${flags_list} -g -O0)
        endif()

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

        target_sources(${test} PRIVATE $<TARGET_OBJECTS:${obj_target}>)
    endforeach()

    add_test(
        NAME ${test}
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/unit"
        COMMAND $<TARGET_FILE:${test}>
    )

    add_dependencies(unit ${test})
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