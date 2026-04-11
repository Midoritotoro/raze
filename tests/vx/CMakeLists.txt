cmake_minimum_required(VERSION 4.1 FATAL_ERROR)

project(razeTestsVx LANGUAGES CXX)

set(raze_DIR "${CMAKE_CURRENT_LIST_DIR}/../../out/build")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

if (PROJECT_IS_TOP_LEVEL)
    find_package(raze REQUIRED COMPONENTS raze)
endif()

enable_testing()
macro(raze_vx_add_test filename)
    add_executable(${filename} ${filename}.cpp ${CMAKE_CURRENT_SOURCE_DIR}/SimdTestTools.h)
    target_compile_options(${filename} PRIVATE /bigobj)
    
    target_link_libraries(
      ${filename} PRIVATE
      raze::raze
    )

    add_test(${filename} ${CMAKE_CURRENT_BINARY_DIR}/"vx"/${filename})
    target_compile_features(${filename} PUBLIC cxx_std_23)
endmacro(raze_vx_add_test)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FILE(GLOB vxTests *.cpp)

FOREACH(testsource ${vxTests})
    GET_FILENAME_COMPONENT(filename ${testsource} NAME_WE)
    raze_vx_add_test(${filename})
ENDFOREACH()