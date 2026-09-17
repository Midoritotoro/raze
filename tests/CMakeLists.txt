cmake_minimum_required(VERSION 4.1  FATAL_ERROR)

project(razeTests LANGUAGES CXX)

set(raze_DIR "${CMAKE_CURRENT_LIST_DIR}/../out/build")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raze)


add_subdirectory(algorithm)
add_subdirectory(math)
add_subdirectory(vx)