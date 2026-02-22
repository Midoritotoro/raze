cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(razeBenchmarks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    add_executable(${filename} ${filename}.cpp ${CMAKE_CURRENT_SOURCE_DIR}/tools/BenchmarkHelper.h)
    target_include_directories(${filename} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tools)

    add_compile_options(-O2)

    target_link_libraries(
      ${filename} PUBLIC
      raze::raze
      benchmark::benchmark
    )

    add_test(${filename} ${CMAKE_CURRENT_BINARY_DIR}/${filename})
endmacro(razeBenchmarks_add_benchmark)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FILE(GLOB razeBenchmarks *.cpp *.h)

FOREACH(benchsource ${razeBenchmarks})
   GET_FILENAME_COMPONENT(filename ${benchsource} NAME_WE)
   razeBenchmarks_add_benchmark(${filename})
ENDFOREACH()
