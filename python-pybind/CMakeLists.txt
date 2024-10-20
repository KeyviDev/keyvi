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

# Add a library using FindPython's tooling (pybind11 also provides a helper like
# this)
FILE(GLOB_RECURSE KEYVI_PYBIND_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
pybind11_add_module(keyvi_scikit_core ${KEYVI_PYBIND_SOURCES})

target_link_libraries(keyvi_scikit_core PRIVATE keyvi)

# This is passing in the version as a define just as an example
target_compile_definitions(keyvi_scikit_core PRIVATE VERSION_INFO=${PROJECT_VERSION})

# The install directory is the output (wheel) directory
install(TARGETS keyvi_scikit_core DESTINATION .)
