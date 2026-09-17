include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/tools/Targets.cmake")

cmake_minimum_required(VERSION 4.1 FATAL_ERROR)
project(razeTestsMath LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

if(PROJECT_IS_TOP_LEVEL)
    set(raze_DIR "${CMAKE_CURRENT_LIST_DIR}/../../out/build")
    find_package(raze REQUIRED COMPONENTS raze)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/tools/GenerateTest.cmake)

enable_testing()
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
   set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
endif()

add_custom_target(math)
add_dependencies(unit math)
raze_glob_unit("math" ${CMAKE_CURRENT_SOURCE_DIR} "fused/*.cpp")
raze_glob_unit("math" ${CMAKE_CURRENT_SOURCE_DIR} "core/*.cpp")
