cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(razeTestsAlgorithm LANGUAGES CXX)

set(raze_DIR "${CMAKE_CURRENT_LIST_DIR}/../../out/build")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

if (PROJECT_IS_TOP_LEVEL)
    find_package(raze REQUIRED COMPONENTS raze)
endif()

enable_testing()

if(MSVC)
    add_compile_options(/Od /Zi /bigobj /D NDEBUG=0) 
else()
    add_compile_options(-O0 -g -D NDEBUG=0)
endif()

macro(raze_algorithm_add_test filename)
    add_executable(${filename} ${filename}.cpp)

    target_link_libraries(
      ${filename} PRIVATE
      raze::raze
    )

    set_target_properties(${filename} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    add_test(NAME ${filename} COMMAND ${filename})
endmacro(raze_algorithm_add_test)

FILE(GLOB algorithmTests *.cpp)

FOREACH(testsource ${algorithmTests})
   GET_FILENAME_COMPONENT(filename ${testsource} NAME_WE)
   raze_algorithm_add_test(${filename})
ENDFOREACH()