cmake_minimum_required(VERSION 3.0.2)
project(keyvi)

#### Build Type
if (CMAKE_BUILD_TYPE)
  string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPER)
endif()

#### Cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

#### Compiler Flags ####

# configure C++11
if(NOT CMAKE_VERSION VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD 11)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# keyvi specific compile options, definitions and flags
set (_KEVYI_COMPILE_OPTIONS "-Wall")
set (_KEYVI_COMPILE_DEFINITIONS "RAPIDJSON_HAS_STDSTRING")

# todo: add check for SSE4.2, make it optional
set (_KEYVI_CXX_FLAGS "-msse4.2")

# OSX specifics
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (_KEYVI_COMPILE_DEFINITIONS "${_KEYVI_COMPILE_DEFINITIONS} OS_MACOSX")
    set (_KEYVI_CXX_FLAGS "${_KEYVI_CXX_FLAGS} -mmacosx-version-min=10.9")
endif ()

# build type specific settings
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} -O0 --coverage")

# link libraries
set(_KEYVI_LINK_LIBRARIES_STATIC "")
set(_KEYVI_LINK_LIBRARIES_DYNAMIC "")

if (CMAKE_BUILD_TYPE_UPPER MATCHES COVERAGE)
    set(_KEYVI_LINK_LIBRARIES_DYNAMIC "${_KEYVI_LINK_LIBRARIES_DYNAMIC} gcov")
    set(_KEYVI_LINK_FLAGS "--coverage")
endif (CMAKE_BUILD_TYPE_UPPER MATCHES COVERAGE)

#### Dependencies ####

# TPIE settings (todo: make optional)
set(COMPILE_TEST OFF CACHE BOOL "")
set(TPIE_PARALLEL_SORT 1 CACHE BOOL "")
add_subdirectory(keyvi/3rdparty/tpie EXCLUDE_FROM_ALL)
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/3rdparty/tpie")
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_BINARY_DIR}/keyvi/3rdparty/tpie")
set(_KEYVI_LINK_LIBRARIES_STATIC "${_KEYVI_LINK_LIBRARIES_STATIC} tpie")

# tiny process library
add_subdirectory(keyvi/3rdparty/tiny-process-library)
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/3rdparty/tiny-process-library/")
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_BINARY_DIR}/keyvi/3rdparty/tiny-process-library/")
set(_KEYVI_LINK_LIBRARIES_STATIC "${_KEYVI_LINK_LIBRARIES_STATIC} tiny-process-library")

# BOOST
set (_KEYVI_BOOST_LIBRARIES "program_options" "iostreams" "filesystem" "system" "regex" "thread")
set (_KEYVI_BOOST_LIBRARIES_TEST "unit_test_framework")
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS ${_KEYVI_BOOST_LIBRARIES} ${_KEYVI_BOOST_LIBRARIES_TEST})
if (Boost_FOUND)
    list(APPEND KEYVI_INCLUDES "${Boost_INCLUDE_DIRS}")
else ()
    message(FATAL_ERROR "Can not find Boost")
endif ()
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(_KEYVI_LINK_LIBRARIES_STATIC "${_KEYVI_LINK_LIBRARIES_STATIC} boost_program_options boost_iostreams boost_filesystem boost_system boost_regex boost_thread-mt")
else ()
    set(_KEYVI_LINK_LIBRARIES_DYNAMIC "${_KEYVI_LINK_LIBRARIES_DYNAMIC} boost_program_options boost_iostreams boost_filesystem boost_system boost_regex boost_thread")
endif ()

# Zlib
find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    IF(ZLIB_VERSION_STRING VERSION_LESS "1.2.8")
        message(FATAL_ERROR "-- ZLIB version too old, please install at least v1.2.8")
    ENDIF(ZLIB_VERSION_STRING VERSION_LESS "1.2.8")
    list(APPEND KEYVI_INCLUDES "${ZLIB_INCLUDE_DIRS}")

else ()
    message(FATAL_ERROR "Can not find ZLib")
endif (ZLIB_FOUND)
if (ZLIB_ROOT OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(_KEYVI_LINK_LIBRARIES_STATIC "${_KEYVI_LINK_LIBRARIES_STATIC} z")
else ()
  set(_KEYVI_LINK_LIBRARIES_DYNAMIC "${_KEYVI_LINK_LIBRARIES_DYNAMIC} z")
endif ()

# snappy
# currently reuses on tpies FindSnappy module
find_package(Snappy REQUIRED)
if(${Snappy_FOUND})
    list(APPEND KEYVI_INCLUDES "${Snappy_INCLUDE_DIR}")

else(${Snappy_FOUND})
    message(FATAL_ERROR "Can not find Snappy")
endif(${Snappy_FOUND})
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(_KEYVI_LINK_LIBRARIES_STATIC "${_KEYVI_LINK_LIBRARIES_STATIC} snappy")
else ()
  set(_KEYVI_LINK_LIBRARIES_DYNAMIC "${_KEYVI_LINK_LIBRARIES_DYNAMIC} snappy")
endif ()

# rapidjson
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/3rdparty/rapidjson/include")

# misc
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/3rdparty/utf8")
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/3rdparty/misc")
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/3rdparty/msgpack-c/include")
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/3rdparty/xchange/src")
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/3rdparty/concurrentqueue")

# keyvi
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/include")


# target_compile_options and target_compile_definitions expect a list
string(REPLACE " " ";" _KEYVI_CXX_FLAGS_LIST "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}} ${_KEVYI_COMPILE_OPTIONS} ${_KEYVI_CXX_FLAGS}")
string(REPLACE " " ";" _KEYVI_COMPILE_DEFINITIONS_LIST "${_KEYVI_COMPILE_DEFINITIONS}")

#### Targets ####

# keyvicompiler
add_executable(keyvicompiler keyvi/bin/keyvicompiler/keyvicompiler.cpp)
target_link_libraries(keyvicompiler tpie ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
target_compile_options(keyvicompiler PRIVATE ${_KEYVI_CXX_FLAGS_LIST})
target_compile_definitions(keyvicompiler PRIVATE ${_KEYVI_COMPILE_DEFINITIONS_LIST})
target_include_directories(keyvicompiler PRIVATE "$<BUILD_INTERFACE:${KEYVI_INCLUDES}>")

install (TARGETS keyvicompiler DESTINATION bin COMPONENT applications OPTIONAL)

# keyviinspector
add_executable(keyviinspector keyvi/bin/keyviinspector/keyviinspector.cpp)
target_link_libraries(keyviinspector tpie ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
target_compile_options(keyviinspector PRIVATE ${_KEYVI_CXX_FLAGS_LIST})
target_compile_definitions(keyviinspector PRIVATE ${_KEYVI_COMPILE_DEFINITIONS_LIST})
target_include_directories(keyviinspector PRIVATE "$<BUILD_INTERFACE:${KEYVI_INCLUDES}>")

install (TARGETS keyviinspector DESTINATION bin COMPONENT applications OPTIONAL)

# keyvimerger
add_executable(keyvimerger keyvi/bin/keyvimerger/keyvimerger.cpp)
target_link_libraries(keyvimerger tpie ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
target_compile_options(keyvimerger PRIVATE ${_KEYVI_CXX_FLAGS_LIST})
target_compile_definitions(keyvimerger PRIVATE ${_KEYVI_COMPILE_DEFINITIONS_LIST})
target_include_directories(keyvimerger PRIVATE "$<BUILD_INTERFACE:${KEYVI_INCLUDES}>")

install (TARGETS keyvimerger DESTINATION bin COMPONENT applications)

# keyvi_c
add_library(keyvi_c SHARED keyvi/bin/keyvi_c/c_api.cpp)
target_link_libraries(keyvi_c tpie ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
target_compile_options(keyvi_c PRIVATE ${_KEYVI_CXX_FLAGS_LIST})
target_compile_definitions(keyvi_c PRIVATE ${_KEYVI_COMPILE_DEFINITIONS_LIST})
target_include_directories(keyvi_c PRIVATE "$<BUILD_INTERFACE:${KEYVI_INCLUDES}>")

# unit tests
FILE(GLOB_RECURSE UNIT_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} keyvi/tests/keyvi/*.cpp)
add_executable(unit_test_all ${UNIT_TEST_SOURCES})
target_link_libraries(unit_test_all tiny-process-library tpie ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
target_compile_options(unit_test_all PRIVATE ${_KEYVI_CXX_FLAGS_LIST})
target_compile_definitions(unit_test_all PRIVATE ${_KEYVI_COMPILE_DEFINITIONS_LIST})
target_include_directories(unit_test_all PRIVATE "$<BUILD_INTERFACE:${KEYVI_INCLUDES}>")
add_dependencies(unit_test_all keyvimerger)

# bindings
add_custom_target(bindings
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tpie> ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tiny-process-library> ${CMAKE_BINARY_DIR}
    DEPENDS tpie tiny-process-library
)

#### Configuration file for integrations and bindings ####

# expose all flags used

# includes
string(REPLACE ";" " " _KEYVI_INCLUDES "${KEYVI_INCLUDES}")

# compile flags
set(_KEYVI_CXX_FLAGS_ALL "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}} ${_KEYVI_CXX_FLAGS} ${_KEVYI_COMPILE_OPTIONS} -std=c++11")

configure_file(keyvi/flags.cmake keyvi/flags)

add_library(keyvi INTERFACE)

target_include_directories(keyvi INTERFACE "$<BUILD_INTERFACE:${KEYVI_INCLUDES}>")
target_compile_definitions(keyvi INTERFACE ${_KEYVI_COMPILE_DEFINITIONS_LIST})
target_link_libraries(keyvi INTERFACE tiny-process-library tpie ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
add_dependencies(keyvi tpie)
