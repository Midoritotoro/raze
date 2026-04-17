cmake_minimum_required (VERSION 4.1  FATAL_ERROR)

project ("raze"
    VERSION 1.0.0
    DESCRIPTION "..."
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR "lib")
endif()
if (NOT DEFINED CMAKE_INSTALL_BINDIR)
  set(CMAKE_INSTALL_BINDIR "bin")
endif()
if (NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
  set(CMAKE_INSTALL_INCLUDEDIR "include")
endif()

option(RAZE_INSTALL "" ${PROJECT_IS_TOP_LEVEL})
option(RAZE_ENABLE_OPENMP "Enable OpenMP support in raze" OFF)


set(RAZE_INSTALL_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/raze")
file(GLOB_RECURSE PRIVATE_SOURCES CONFIGURE_DEPENDS "src/raze/*.cpp" "src/raze/*.c")
file(GLOB_RECURSE PRIVATE_HEADERS CONFIGURE_DEPENDS "src/raze/*.h" "src/raze/*.hpp" "src/raze/*.inl")
file(GLOB_RECURSE PUBLIC_HEADERS CONFIGURE_DEPENDS "include/raze/*.h" "include/raze/*.hpp" "include/raze/*.inl")

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)

add_library(raze STATIC ${PRIVATE_SOURCES} ${PRIVATE_HEADERS} ${PUBLIC_HEADERS})
add_library(raze::raze ALIAS raze)

target_sources(raze PRIVATE ${PRIVATE_SOURCES} ${PRIVATE_HEADERS})

target_include_directories(raze PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_target_properties(
    raze PROPERTIES
    
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD 23
    CXX_EXTENSIONS OFF

    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"

    EXPORT_NAME raze
    OUTPUT_NAME raze

    LINKER_LANGUAGE CXX
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_features(raze PUBLIC cxx_std_23)
    set(CMAKE_DEBUG_TARGET_PROPERTIES "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(CMAKE_GENERATOR MATCHES "Visual Studio")
	if((CMAKE_VS_PLATFORM_NAME STREQUAL "x64") OR (CMAKE_VS_PLATFORM_NAME STREQUAL "x86"))
		include(cmake/x86_Simd.cmake)
    endif()
endif()


if((CMAKE_SYSTEM_PROCESSOR STREQUAL "i686") OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64") OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64"))
		include(cmake/x86_Simd.cmake)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -Ob3) 
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O2 -GL-)
    endif()

    target_link_options(raze PRIVATE "/LTCG;/opt:ref,icf;")

    target_compile_options(
        raze 
        PRIVATE
            /bigobj
            /permissive-
            /W4
            
            /utf-8

            /MP
            /EHsc

            /guard:cf

            /w15038 
            /w14265 
            /wd4018
            /wd4100
            /wd4242
            /wd4244
            /wd4245
            /wd4267
            /wd4305
            /wd4324
            /wd4389
            /wd4456
            /wd4457
            /wd4458
            /wd4459
            /wd4611
            /wd4702
            /wd4828

            /Zi
            /Zc:__cplusplus
    )   
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    
    target_compile_options(
        raze 
        PRIVATE
            /permissive-
            /W4
            /utf-8
            /MP
            /O3
            /EHsc
            /Zi
            /Zc:__cplusplus
        )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(
        raze 
        PRIVATE
            /utf-8

            /O3
            /W4

            /EHsc   

            /Zi
            /Zc:__cplusplus
    )
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

set(RAZE_USER_FORCE_ENABLED OFF)

foreach(flag ${RAZE_FORCE_LIST})
    if(DEFINED ${flag})
        set(RAZE_USER_FORCE_ENABLED ON)
    endif()
endforeach()

function(raze_test_msvc_arch flag result_var) 
    set(src "${CMAKE_BINARY_DIR}/simd_test_${flag}.cpp") 
    file(WRITE "${src}" "int main(){return 0;}") 

    try_compile(${result_var} 
        "${CMAKE_BINARY_DIR}" 
        "${src}" CMAKE_FLAGS 
        "-DCMAKE_CXX_FLAGS=/arch:${flag}" 
        OUTPUT_VARIABLE out) 
endfunction()

if(RAZE_USER_FORCE_ENABLED)
    message(STATUS "RAZE: ISA FORCE mode enabled")

    if(MSVC)

        message(STATUS "RAZE: MSVC detected — validating /arch: flags")

        if(RAZE_FORCE_SSE2)
            add_compile_definitions(RAZE_FORCE_SSE2=1)
        endif()

        if(RAZE_FORCE_AVX)
            raze_test_msvc_arch("AVX" AVX_OK)
            if(NOT AVX_OK)
                message(FATAL_ERROR "MSVC does not support /arch:AVX")
            endif()
            add_compile_options(/arch:AVX)
            add_compile_definitions(RAZE_FORCE_AVX=1)
        endif()

        if(RAZE_FORCE_AVX2)
            raze_test_msvc_arch("AVX2" AVX2_OK)
            if(NOT AVX2_OK)
                message(FATAL_ERROR "MSVC does not support /arch:AVX2")
            endif()
            add_compile_options(/arch:AVX2)
            add_compile_definitions(RAZE_FORCE_AVX2=1)
        endif()

        if(RAZE_FORCE_FMA3)
            raze_test_msvc_arch("AVX2" AVX2_OK)
            if(NOT AVX2_OK)
                message(FATAL_ERROR "MSVC does not support /arch:AVX2")
            endif()
            add_compile_options(/arch:AVX2)
            add_compile_definitions(RAZE_FORCE_FMA3=1)
        endif()


       if(RAZE_FORCE_AVX512F OR RAZE_FORCE_AVX512BW OR RAZE_FORCE_AVX512CD
            OR RAZE_FORCE_AVX512DQ OR RAZE_FORCE_AVX512VL
            OR RAZE_FORCE_AVX512VBMI OR RAZE_FORCE_AVX512VBMI2)

            raze_test_msvc_arch("AVX512" AVX512_OK)
            if(NOT AVX512_OK)
                message(FATAL_ERROR "MSVC does not support /arch:AVX512")
            endif()

            add_compile_options(/arch:AVX512)

            foreach(flag RAZE_FORCE_AVX512F
                         RAZE_FORCE_AVX512BW
                         RAZE_FORCE_AVX512CD
                         RAZE_FORCE_AVX512DQ
                         RAZE_FORCE_AVX512VL
                         RAZE_FORCE_AVX512VBMI
                         RAZE_FORCE_AVX512VBMI2)
                    if(${flag})
                        add_compile_definitions(${flag}=1)
                    endif()
                endforeach()
            endif()
        else()
            include(CheckCXXCompilerFlag)

            macro(raze_require flag compileflag)
                if(${flag})
                    check_cxx_compiler_flag(${compileflag} ${flag}_SUPPORTED)
                    if(NOT ${flag}_SUPPORTED)
                        message(FATAL_ERROR "${flag}=1 requires ${compileflag}, but compiler does not support it")
                    endif()
                    add_compile_options(${compileflag})
                    add_compile_definitions(${flag}=1)
                endif()
            endmacro()

            raze_require(RAZE_FORCE_SSE2        "-msse2")
            raze_require(RAZE_FORCE_SSE3        "-msse3")
            raze_require(RAZE_FORCE_SSSE3       "-mssse3")
            raze_require(RAZE_FORCE_SSE41       "-msse4.1")
            raze_require(RAZE_FORCE_SSE42       "-msse4.2")
            raze_require(RAZE_FORCE_AVX         "-mavx")
            raze_require(RAZE_FORCE_AVX2        "-mavx2")
            raze_require(RAZE_FORCE_FMA3        "-mfma")
            raze_require(RAZE_FORCE_AVX512F     "-mavx512f")
            raze_require(RAZE_FORCE_AVX512BW    "-mavx512bw")
            raze_require(RAZE_FORCE_AVX512CD    "-mavx512cd")
            raze_require(RAZE_FORCE_AVX512DQ    "-mavx512dq")
            raze_require(RAZE_FORCE_AVX512VL    "-mavx512vl")
            raze_require(RAZE_FORCE_AVX512VBMI  "-mavx512vbmi")
            raze_require(RAZE_FORCE_AVX512VBMI2 "-mavx512vbmi2")

    endif()

    add_compile_definitions(RAZE_ISA_FORCE_ENABLED=1)

else()

    message(STATUS "RAZE: No FORCE ISA specified — using runtime dispatch")
    add_compile_definitions(RAZE_ISA_FORCE_ENABLED=0)

endif()


if (RAZE_ENABLE_OPENMP)
    find_package(OpenMP REQUIRED)

    if(OpenMP_CXX_FOUND)
	    include_directories(${OpenMP_INCLUDE_DIRS})
        message("OpenMP FOUND dir is ${OpenMP_INCLUDE_DIRS}.")

        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") 
            set(CMAKE_CXX_FLAGS "/openmp")
        else()
            set(CMAKE_CXX_FLAGS "-fopenmp")
        endif()

        add_compile_definitions(RAZE_ENABLE_OPENMP=1)
        target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)
    endif()
endif()

if(PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(benchmarks)
endif()

if(RAZE_INSTALL AND NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/Util_InstallRules.cmake)
endif()
