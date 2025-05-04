# dependencies (in ubuntu 24.04 installation):
# * clang++ version 18.1.3
# * libglm-dev/noble,noble,now 0.9.9.8+ds-7
# * libtbb-dev/noble,now 2021.11.0-2ubuntu2
# * libsdl2-dev/noble,now 2.30.0+dfsg-1build3
# * libsdl2-gfx-dev/noble,now 1.0.4+dfsg-5build1
# * libsdl2-image-dev/noble,now 2.8.2+dfsg-1build2
# * libsdl2-ttf-dev/noble,now 2.22.0+dfsg-1
# * opengl related packages

cmake_minimum_required(VERSION 3.10)
project(glosi)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build")
file(MAKE_DIRECTORY ${BUILD_DIR})
set(CMAKE_CXX_STANDARD 23)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(TBB REQUIRED)
add_compile_options(-O3 -march=native)
add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
add_compile_options(-Wno-changes-meaning -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # workaround for issue when g++ optimizes away stores before a 'new in place'
    #  falsely assuming that all object fields are initialized by the constructor
    add_compile_options(-flifetime-dse=1)
endif()
add_executable(glosi src/main.cpp)
target_link_libraries(glosi SDL2::SDL2 SDL2_image SDL2_ttf OpenGL::OpenGL glm::glm TBB::tbb)
