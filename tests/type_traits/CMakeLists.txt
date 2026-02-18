cmake_minimum_required(VERSION 4.1  FATAL_ERROR)

project(razeTestsTypeTraits LANGUAGES CXX)


set(raze_DIR "C:/Users/danya/source/repos/Midoritotoro/raze/out/build")

if (PROJECT_IS_TOP_LEVEL)
    find_package(raze REQUIRED COMPONENTS
        raze 
   )
endif()

enable_testing()
macro(raze_type_traits_add_test filename)
    add_executable(${filename} ${filename}.cpp)

    set(CMAKE_CXX_FLAGS "-march=native")
    target_link_libraries(
      ${filename} PRIVATE
      raze::raze
    )

    add_test(${filename} ${CMAKE_CURRENT_BINARY_DIR}/"algorithm"/${filename})
    target_compile_features(${filename} PUBLIC cxx_std_23)
endmacro(raze_type_traits_add_test)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FILE(GLOB typeTraitsTests *.cpp)

FOREACH(testsource ${typeTraitsTests})
    GET_FILENAME_COMPONENT(filename ${testsource} NAME_WE)
    raze_type_traits_add_test(${filename})
ENDFOREACH()