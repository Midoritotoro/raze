cmake_minimum_required(VERSION 4.1 FATAL_ERROR)

project(razeTestsAlgorithm LANGUAGES CXX)


set(raze_DIR "C:/Users/danya/source/repos/Midoritotoro/raze/out/build")

if (PROJECT_IS_TOP_LEVEL)
    find_package(raze REQUIRED COMPONENTS raze)
endif()

# target_compile_definitions(raze PUBLIC RAZE_FORCE_AVX512BW)

enable_testing()
macro(raze_algorithm_add_test filename)
    add_executable(${filename} ${filename}.cpp)

    add_compile_options(/bigobj /arch:AVX512 /GL-)
    
    target_link_libraries(
      ${filename} PRIVATE
      raze::raze
    )

    add_test(${filename} ${CMAKE_CURRENT_BINARY_DIR}/"algorithm"/${filename})
    target_compile_features(${filename} PUBLIC cxx_std_23)
endmacro(raze_algorithm_add_test)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FILE(GLOB algorithmTests *.cpp)

FOREACH(testsource ${algorithmTests})
    GET_FILENAME_COMPONENT(filename ${testsource} NAME_WE)
    raze_algorithm_add_test(${filename})
ENDFOREACH()