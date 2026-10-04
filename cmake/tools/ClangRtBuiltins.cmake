function(raze_link_clang_rt_builtins tgt)
    if(NOT WIN32 OR NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        return()
    endif()

    set(_resdir "")

    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        execute_process(
            COMMAND "${CMAKE_CXX_COMPILER}" /clang:--print-resource-dir
            OUTPUT_VARIABLE _resdir
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE _rc
        )
    endif()

    if(NOT _resdir)
        execute_process(
            COMMAND "${CMAKE_CXX_COMPILER}" --print-resource-dir
            OUTPUT_VARIABLE _resdir
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
    endif()

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_names clang_rt.builtins-x86_64)
        set(_arch  x86_64)
    else()
        set(_names clang_rt.builtins-i386)
        set(_arch  i386)
    endif()

    find_library(RAZE_CLANG_RT_BUILTINS
        NAMES ${_names}
        HINTS
            "${_resdir}/lib/windows"
            "${_resdir}/lib/${_arch}-pc-windows-msvc"
            "${_resdir}/lib"
        NO_DEFAULT_PATH
    )

    if(NOT RAZE_CLANG_RT_BUILTINS AND _resdir)
        file(GLOB _glob
            "${_resdir}/lib/*/clang_rt.builtins-${_arch}.lib"
            "${_resdir}/lib/windows/clang_rt.builtins-${_arch}.lib"
        )
        if(_glob)
            list(GET _glob 0 RAZE_CLANG_RT_BUILTINS)
        endif()
    endif()

    if(RAZE_CLANG_RT_BUILTINS)
        target_link_libraries(${tgt} INTERFACE "${RAZE_CLANG_RT_BUILTINS}")
        message(STATUS "RAZE: target_clones runtime -> ${RAZE_CLANG_RT_BUILTINS}")
    else()
        target_compile_definitions(${tgt} INTERFACE RAZE_PROVIDE_CPU_MODEL=1)
        message(WARNING
            "RAZE: clang_rt.builtins not found; "
            "falling back to built-in __cpu_model shim"
        )
    endif()
endfunction()