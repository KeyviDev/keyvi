cmake_minimum_required(VERSION 3.15...3.27)

# Scikit-build-core sets these values for you, or you can just hard-code the
# name and version.
project(
  ${SKBUILD_PROJECT_NAME}
  VERSION ${SKBUILD_PROJECT_VERSION}
  LANGUAGES CXX)

# Find the module development requirements (requires FindPython from 3.17 or
# scikit-build-core's built-in backport)
find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

# Use _keyvi_pybind as internal target name to avoid conflict with the keyvi C++ library target
FILE(GLOB_RECURSE KEYVI_PYBIND_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
pybind11_add_module(_keyvi_pybind ${KEYVI_PYBIND_SOURCES})

set_target_properties(_keyvi_pybind PROPERTIES OUTPUT_NAME keyvi)
target_include_directories(_keyvi_pybind PRIVATE src)
target_link_libraries(_keyvi_pybind PRIVATE keyvi)

# This is passing in the version as a define just as an example
target_compile_definitions(_keyvi_pybind PRIVATE VERSION_INFO=${PROJECT_VERSION})

# The install directory is the output (wheel) directory
install(TARGETS _keyvi_pybind DESTINATION .)
install(FILES keyvimerger.py DESTINATION .)
