# note: for dependencies in ubuntu and arch linux see `make.sh`
cmake_minimum_required(VERSION 3.10)
project(glosi)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build")
file(MAKE_DIRECTORY ${BUILD_DIR})
set(CMAKE_CXX_STANDARD 26)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(TBB REQUIRED)
add_compile_options(-O3)
add_compile_options(-g)
add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
add_compile_options(-Wno-unused-variable -Wno-unused-function -Wno-unused-parameter)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # workaround for issue when compiler optimizes away stores before new in place
    #  falsely assuming that all object fields are initialized by constructor resulting in crash
    add_compile_options(-flifetime-dse=1)

    # allow class member names same as class name
    add_compile_options(-Wno-changes-meaning)
endif()
add_executable(${PROJECT_NAME} src/main.cpp)
# link against the SDL3 targets provided by the packages
# SDL3_image and SDL3_ttf export plain library names, so link them directly
# SDL3::SDL3 is the imported target for the core library
target_link_libraries(${PROJECT_NAME} SDL3::SDL3 SDL3_image SDL3_ttf OpenGL::OpenGL glm::glm TBB::tbb)
