cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
project(xchange)

set(JSONCPP_WITH_TESTS OFF CACHE BOOL "Turn off JsonCpp tests")
set(MSGPACK_CXX11 ON CACHE BOOL "Turn on C++11 for msgpack")
set(MSGPACK_BUILD_EXAMPLES OFF CACHE BOOL "Turn off msgpack examples")
set(MSGPACK_BUILD_TESTS OFF CACHE BOOL "Turn off msgpack tests")
set(MSGPACK_ENABLE_SHARED OFF CACHE BOOL "Turn off msgpack shared library")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(formats/jsoncpp)
add_subdirectory(formats/msgpack)

include_directories(
	src
	formats/msgpack/include
	formats/rapidjson/include
	formats/jsoncpp/include
)

link_directories(${CMAKE_BINARY_DIR}/lib)

list(APPEND SOURCES
	src/main.cpp
	src/msgpack/type/rapidjson.hpp
	src/msgpack/type/jsoncpp.hpp
)

add_definitions(-std=c++11)


add_executable(xchange ${SOURCES})

target_link_libraries(xchange msgpack-static jsoncpp_lib_static)

if (MSVC)
    set_property(TARGET xchange APPEND_STRING PROPERTY COMPILE_FLAGS "/wd4290")
    set_property(TARGET msgpack APPEND_STRING PROPERTY COMPILE_FLAGS "/wd4100 /wd4127 /wd4204 /wd4290")
    set_property(TARGET msgpack-static APPEND_STRING PROPERTY COMPILE_FLAGS "/wd4100 /wd4127 /wd4204 /wd4290")
endif (MSVC)
