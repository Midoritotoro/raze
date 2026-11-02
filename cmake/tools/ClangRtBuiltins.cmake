function(raze_link_clang_rt_builtins tgt)
    if(NOT WIN32)
        return()
    endif()

    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "(Clang|IntelLLVM|Intel)")
        return()
    endif()

    set(RAZE_CLANG_RT_BUILTINS "" CACHE FILEPATH "Path to clang_rt.builtins")

    set(_clang_like_compiler "")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
        set(_clang_like_compiler "${CMAKE_CXX_COMPILER}")
    endif()

    if(NOT _clang_like_compiler)
        find_program(RAZE_ICPX_COMPILER
            NAMES icpx icpx.exe
            HINTS
                "$ENV{ONEAPI_ROOT}/compiler/latest/bin"
                "$ENV{ONEAPI_ROOT}/compiler/latest/windows/bin"
                "$ENV{CMPLR_ROOT}/bin"
                "$ENV{CMPLR_ROOT}/windows/bin"
            PATHS
                "C:/Program Files (x86)/Intel/oneAPI/compiler/latest/bin"
                "C:/Program Files/Intel/oneAPI/compiler/latest/bin"
            NO_DEFAULT_PATH
        )
        if(RAZE_ICPX_COMPILER)
            set(_clang_like_compiler "${RAZE_ICPX_COMPILER}")
            message(STATUS "RAZE: icpx compiler found -> ${RAZE_ICPX_COMPILER}")
        endif()
    endif()

    if(NOT _clang_like_compiler)
        set(_clang_like_compiler "${CMAKE_CXX_COMPILER}")
    endif()

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_builtin_names
            "clang_rt.builtins-x86_64.lib"
            "libclang_rt.builtins-x86_64.lib"
        )
        set(_arch x86_64)
    else()
        set(_builtin_names
            "clang_rt.builtins-i386.lib"
            "libclang_rt.builtins-i386.lib"
        )
        set(_arch i386)
    endif()

    foreach(_name IN LISTS _builtin_names)
        execute_process(
            COMMAND "${_clang_like_compiler}" "--print-file-name=${_name}"
            OUTPUT_VARIABLE _full_path
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE _rc
        )
        if(_rc EQUAL 0 AND _full_path AND EXISTS "${_full_path}")
            set(RAZE_CLANG_RT_BUILTINS "${_full_path}" CACHE FILEPATH "" FORCE)
            break()
        endif()
    endforeach()

    if(NOT RAZE_CLANG_RT_BUILTINS)
        set(_resdir "")
        execute_process(
            COMMAND "${_clang_like_compiler}" --print-resource-dir
            OUTPUT_VARIABLE _resdir
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE _rc
        )
        if(NOT _rc EQUAL 0 OR NOT _resdir)
            execute_process(
                COMMAND "${_clang_like_compiler}" /clang:--print-resource-dir
                OUTPUT_VARIABLE _resdir
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
        endif()
    endif()

    if(NOT RAZE_CLANG_RT_BUILTINS)
        set(_search_hints "")
        if(_resdir)
            list(APPEND _search_hints
                "${_resdir}/lib/windows"
                "${_resdir}/lib/${_arch}-pc-windows-msvc"
                "${_resdir}/lib"
            )
        endif()
        list(APPEND _search_hints
            "$ENV{ONEAPI_ROOT}/compiler/latest/lib/clang/*/lib/windows"
            "$ENV{CMPLR_ROOT}/lib/clang/*/lib/windows"
        )

        find_library(RAZE_CLANG_RT_BUILTINS
            NAMES clang_rt.builtins-${_arch} libclang_rt.builtins-${_arch}
            HINTS ${_search_hints}
            NO_DEFAULT_PATH
        )

        if(NOT RAZE_CLANG_RT_BUILTINS)
            file(GLOB_RECURSE _candidates
                "${_resdir}/lib/*/clang_rt.builtins-${_arch}.lib"
                "${_resdir}/lib/*/libclang_rt.builtins-${_arch}.lib"
                "$ENV{ONEAPI_ROOT}/compiler/latest/lib/clang/*/lib/windows/clang_rt.builtins-${_arch}.lib"
                "$ENV{ONEAPI_ROOT}/compiler/latest/lib/clang/*/lib/windows/libclang_rt.builtins-${_arch}.lib"
            )
            if(_candidates)
                list(GET _candidates 0 RAZE_CLANG_RT_BUILTINS)
                set(RAZE_CLANG_RT_BUILTINS "${RAZE_CLANG_RT_BUILTINS}" CACHE FILEPATH "" FORCE)
            endif()
        endif()
    endif()

    if(RAZE_CLANG_RT_BUILTINS)
        target_link_libraries(${tgt} INTERFACE "${RAZE_CLANG_RT_BUILTINS}")
        target_compile_definitions(${tgt} INTERFACE RAZE_HAS_CLANG_RT_BUILTINS=1)
        message(STATUS "RAZE: target_clones runtime -> ${RAZE_CLANG_RT_BUILTINS}")
    else()
        target_compile_definitions(${tgt} INTERFACE RAZE_HAS_CLANG_RT_BUILTINS=0)
        message(STATUS "RAZE: clang_rt.builtins not found; ")
    endif()
endfunction()