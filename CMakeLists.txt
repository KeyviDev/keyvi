cmake_minimum_required(VERSION 3.9)
project(keyvi)

EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message( STATUS "Architecture: ${ARCHITECTURE}" )

#### Build Type
if (CMAKE_BUILD_TYPE)
  string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPER)
endif()

#### Cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

#### Compiler Flags ####

# configure C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# keyvi specific compile options, definitions and flags
set (_KEVYI_COMPILE_OPTIONS "-Wall")
set (_KEYVI_COMPILE_DEFINITIONS "RAPIDJSON_HAS_STDSTRING")

if(${ARCHITECTURE} STREQUAL "x86_64")
    set (_KEYVI_CXX_FLAGS "-msse4.2")
    message( STATUS "Architecture: ${ARCHITECTURE} detected: added -msse4.2 compile flag")
endif()

set (_OS_LIBRARIES "")

# OSX specifics
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (_KEYVI_COMPILE_DEFINITIONS "${_KEYVI_COMPILE_DEFINITIONS} OS_MACOSX")
    set (_KEYVI_CXX_FLAGS "${_KEYVI_CXX_FLAGS} -mmacosx-version-min=10.9")
endif()

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

if (WIN32)
  # required for libboost_zlib
  # disable autolinking in boost
  add_definitions( -DBOOST_ALL_NO_LIB )
  add_definitions( -DBOOST_ALL_DYN_LINK )
  link_directories (${Boost_LIBRARY_DIRS})
  
  # required for endian specific functions
  list(APPEND _OS_LIBRARIES "ws2_32")
endif (WIN32)

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

# utfcpp
add_subdirectory(keyvi/3rdparty/utfcpp EXCLUDE_FROM_ALL)
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/3rdparty/utfcpp/source")
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_BINARY_DIR}/keyvi/3rdparty/utfcpp/")

# misc
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/3rdparty/misc")
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/3rdparty/msgpack-c/include")
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/3rdparty/concurrentqueue")

# keyvi
list(APPEND KEYVI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/keyvi/include")


# target_compile_options and target_compile_definitions expect a list
string(REPLACE " " ";" _KEYVI_CXX_FLAGS_LIST "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}} ${_KEVYI_COMPILE_OPTIONS} ${_KEYVI_CXX_FLAGS}")
string(REPLACE " " ";" _KEYVI_COMPILE_DEFINITIONS_LIST "${_KEYVI_COMPILE_DEFINITIONS}")

#### Targets ####

# keyvicompiler
add_executable(keyvicompiler keyvi/bin/keyvicompiler/keyvicompiler.cpp)
target_link_libraries(keyvicompiler ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${Snappy_LIBRARY} ${_OS_LIBRARIES})
target_compile_options(keyvicompiler PRIVATE ${_KEYVI_CXX_FLAGS_LIST})
target_compile_definitions(keyvicompiler PRIVATE ${_KEYVI_COMPILE_DEFINITIONS_LIST})
target_include_directories(keyvicompiler PRIVATE "$<BUILD_INTERFACE:${KEYVI_INCLUDES}>")

install (TARGETS keyvicompiler DESTINATION bin COMPONENT applications OPTIONAL)

# keyviinspector
add_executable(keyviinspector keyvi/bin/keyviinspector/keyviinspector.cpp)
target_link_libraries(keyviinspector ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${Snappy_LIBRARY} ${_OS_LIBRARIES})
target_compile_options(keyviinspector PRIVATE ${_KEYVI_CXX_FLAGS_LIST})
target_compile_definitions(keyviinspector PRIVATE ${_KEYVI_COMPILE_DEFINITIONS_LIST})
target_include_directories(keyviinspector PRIVATE "$<BUILD_INTERFACE:${KEYVI_INCLUDES}>")

install (TARGETS keyviinspector DESTINATION bin COMPONENT applications OPTIONAL)

# keyvimerger
add_executable(keyvimerger keyvi/bin/keyvimerger/keyvimerger.cpp)
target_link_libraries(keyvimerger ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${Snappy_LIBRARY} ${_OS_LIBRARIES})
target_compile_options(keyvimerger PRIVATE ${_KEYVI_CXX_FLAGS_LIST})
target_compile_definitions(keyvimerger PRIVATE ${_KEYVI_COMPILE_DEFINITIONS_LIST})
target_include_directories(keyvimerger PRIVATE "$<BUILD_INTERFACE:${KEYVI_INCLUDES}>")

install (TARGETS keyvimerger DESTINATION bin COMPONENT applications)

# keyvi_c
add_library(keyvi_c SHARED keyvi/bin/keyvi_c/c_api.cpp)
target_link_libraries(keyvi_c ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${Snappy_LIBRARY} ${_OS_LIBRARIES})
target_compile_options(keyvi_c PRIVATE ${_KEYVI_CXX_FLAGS_LIST})
target_compile_definitions(keyvi_c PRIVATE ${_KEYVI_COMPILE_DEFINITIONS_LIST})
target_include_directories(keyvi_c PRIVATE "$<BUILD_INTERFACE:${KEYVI_INCLUDES}>")

# unit tests
FILE(GLOB_RECURSE UNIT_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} keyvi/tests/keyvi/*.cpp)
add_executable(unit_test_all ${UNIT_TEST_SOURCES})
target_link_libraries(unit_test_all tiny-process-library ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${Snappy_LIBRARY} ${_OS_LIBRARIES})
target_compile_options(unit_test_all PRIVATE ${_KEYVI_CXX_FLAGS_LIST})
target_compile_definitions(unit_test_all PRIVATE ${_KEYVI_COMPILE_DEFINITIONS_LIST})
target_include_directories(unit_test_all PRIVATE "$<BUILD_INTERFACE:${KEYVI_INCLUDES}>")
add_dependencies(unit_test_all keyvimerger)

if (WIN32)
  message(STATUS "zlib: ${ZLIB_LIBRARY_RELEASE}")
  # copies the dlls required to run to the build folder
  foreach(LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE} ${Boost_FILESYSTEM_LIBRARY_RELEASE} ${ZLIB_LIBRARY_RELEASE})
    get_filename_component(UTF_BASE_NAME ${LIB} NAME_WE)
    get_filename_component(UTF_PATH ${LIB} PATH)
	if(EXISTS "${UTF_PATH}/${UTF_BASE_NAME}.dll")
      add_custom_command(TARGET unit_test_all POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${UTF_PATH}/${UTF_BASE_NAME}.dll" ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}
        )
	# zlib might be stored in a different folder
	elseif(EXISTS "${UTF_PATH}/../bin/${UTF_BASE_NAME}.dll")
      add_custom_command(TARGET unit_test_all POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${UTF_PATH}/../bin/${UTF_BASE_NAME}.dll" ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}
        )	
	endif()
  endforeach()
endif (WIN32)

# bindings
add_custom_target(bindings
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tiny-process-library> ${CMAKE_BINARY_DIR}
    DEPENDS tiny-process-library
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
target_link_libraries(keyvi INTERFACE tiny-process-library ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${Snappy_LIBRARY} ${_OS_LIBRARIES})

### docs

# don't run it as part of a non-toplevel build, e.g. python
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/sphinx-docs)
    find_package(Doxygen)
    find_package(Sphinx COMPONENTS breathe)

    if(Doxygen_FOUND AND Sphinx_FOUND)
        message ("-- Found doxygen/sphinx, creating sphinx_doc target")
        file(GLOB_RECURSE KEYVI_PUBLIC_HEADERS keyvi/include/keyvi/*.h)

        set(DOXYGEN_GENERATE_LATEX NO)
        set(DOXYGEN_GENERATE_HTML NO)
        set(DOXYGEN_GENERATE_XML YES)
        set(DOXYGEN_INLINE_INHERITED_MEMB YES)

        # for scanning the python extension
        set(SPHINX_EXTENSIONS sphinx.ext.autodoc)
        set(SPHINX_CONF_IMPORT sphinx-docs/conf_extra.py)

        doxygen_add_docs(doxygen_keyvi ${KEYVI_PUBLIC_HEADERS})
        sphinx_add_docs(
        sphinx_doc
        BREATHE_PROJECTS doxygen_keyvi
        BUILDER html
        SOURCE_DIRECTORY sphinx-docs)
    else()
        message ("-- Skip doc target, doxygen/sphinx not found")
    endif()
endif()
