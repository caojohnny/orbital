cmake_minimum_required(VERSION 3.16)
project(orbital VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

find_package(SDL2 REQUIRED)

add_executable(orbital main.c)
target_include_directories(orbital
        PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(orbital
        PRIVATE ${SDL2_LIBRARIES})
