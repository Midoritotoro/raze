cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(razeBenchmarks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)


if (MSVC)
    add_compile_options(/O2 /GS- /DNDEBUG /Oy /bigobj /GL) 
    add_link_options(/LTCG /OPT:REF /OPT:ICF)
endif()

if (PROJECT_IS_TOP_LEVEL)
    find_package(raze REQUIRED)
endif()

enable_testing()
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks
)
 
set(BENCHMARK_ENABLE_DOXYGEN OFF)
set(BENCHMARK_ENABLE_INSTALL OFF)
set(BENCHMARK_ENABLE_TESTING OFF)

add_subdirectory(benchmark EXCLUDE_FROM_ALL)

macro(razeBenchmarks_add_benchmark filename)
    add_executable(${filename} ${filename}.cpp)

    target_include_directories(${filename} PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/tools
    )

    target_link_libraries(
      ${filename} PRIVATE
      raze::raze
      benchmark::benchmark_main
    )

    target_compile_options(${filename} PRIVATE /O2 /Ob2)
    
    add_test(NAME ${filename} COMMAND ${filename})
endmacro(razeBenchmarks_add_benchmark)

file(GLOB BENCH_SOURCES "*.cpp")

foreach(benchsource ${BENCH_SOURCES})
   get_filename_component(filename ${benchsource} NAME_WE)
   if(NOT ${filename} STREQUAL "main") 
       razeBenchmarks_add_benchmark(${filename})
   endif()
endforeach()