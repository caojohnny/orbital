cmake_minimum_required(VERSION 3.16)
project(orbital VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
include("cmake/DownloadCglm.cmake")

add_executable(orbital main.c
        shader_util.c shader_util.h
        system.c system.h)
target_include_directories(orbital
        PRIVATE ${SDL2_INCLUDE_DIRS}
        PRIVATE ${OPENGL_INCLUDE_DIRS}
        PRIVATE ${GLEW_INCLUDE_DIRS})
target_link_libraries(orbital
        PRIVATE ${SDL2_LIBRARIES}
        PRIVATE ${OPENGL_LIBRARIES}
        PRIVATE ${GLEW_LIBRARIES}
        PRIVATE cglm
        PRIVATE m)

