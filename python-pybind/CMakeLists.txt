cmake_minimum_required(VERSION 3.4...3.18)
project(keyvi_pybind11)

add_subdirectory(pybind11)
add_subdirectory(keyvi_cpp)

FILE(GLOB_RECURSE KEYVI_PYBIND_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
pybind11_add_module(keyvi_pybind11 ${KEYVI_PYBIND_SOURCES})

# KEYVI_VERSION_INFO is defined by setup.py and passed into the C++ code as a
# define (VERSION_INFO) here.
target_compile_definitions(keyvi_pybind11
                           PRIVATE VERSION_INFO=${KEYVI_VERSION_INFO})
target_link_libraries(keyvi_pybind11 PRIVATE keyvi)
