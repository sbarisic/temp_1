cmake_minimum_required(VERSION 3.1.0)
project(Core3 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build/)

include_directories(include src)

file(GLOB_RECURSE SOURCES src/*.c src/*.cpp)

add_executable(Core3 ${SOURCES})
