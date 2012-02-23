cmake_minimum_required (VERSION 2.8)
project (align)
option( build_tests "Build all unit tests" )

set( align_SRCS align.cpp text.cpp dictionary.cpp string_impl.cpp )
set( exec_SRCS main.cpp ${align_SRCS} )

# set module path that find modules from the source tree can be used
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules" )

# set version number
# XXX needs a point in the source where this is configured
set (align_VERSION_MAJOR 0)
set (align_VERSION_MINOR 1)

# this crap is used for testing
set (align_DICT_BASE "${PROJECT_SOURCE_DIR}/test_data/dict/")
set (align_FILE_BASE "${PROJECT_SOURCE_DIR}/test_data")
set (align_TEST_E "21_(Ba).txt")
set (align_TEST_F "65_(M21).txt")

# this is for stuff that can be set by cmake
configure_file (
    "${PROJECT_SOURCE_DIR}/align_config.h.in"
    "${PROJECT_BINARY_DIR}/align_config.h"
    )

# os introspection
find_package( Boost 1.48 REQUIRED ) # not yet used, for po
find_package( ICU 4.8 REQUIRED )    # w.i.p making it string impl agnostic
# TODO :   bi_sim

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag( -std=c++0x SUPPORTS_CXX_0x )
message( STATUS "Checking if compiler supports C++0x extensions..." )

find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
        "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY )
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation..."
        )
endif(DOXYGEN_FOUND)

# this is gcc specific, but c++0x declaration is important
# TODO research how msvc does this stuff, or else how cmake
# can do it platform independently
set ( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -std=c++0x" )
# for debug target - -DCMAKE_BUILD_TYPE=Debug
set ( CMAKE_CXX_FLAGS_DEBUG "-g -pg" )


# normal executable
include_directories( ${ICU_INCLUDE_DIRS} ${PROJECT_BINARY_DIR} )
add_executable( align ${exec_SRCS} )
target_link_libraries( align ${ICU_LIBRARIES} )

# test target - -Dbuild_tests=1
if( build_tests )
    enable_testing()
    add_executable( text_test
        text_unittest.cpp ${align_SRCS} )
    target_link_libraries( text_test gtest gtest_main ${ICU_LIBRARIES} )
    add_test( text-tests text_test )
endif()

install (TARGETS align DESTINATION /home/anselm/bin)

