cmake_minimum_required(VERSION 3.1.0)
project(Core3 VERSION 0.1.0 LANGUAGES C CXX)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build/)

include_directories(include src)

file(GLOB_RECURSE SOURCES src/*.c src/*.cpp)

# target_compile_options(Core3 -Wall -Werror)


add_compile_options("-Wall" )
add_compile_options($<$<COMPILE_LANGUAGE:C>:-std=gnu99>)



add_executable(Core3 ${SOURCES})
