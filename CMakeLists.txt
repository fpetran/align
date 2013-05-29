# Master build file for palign
# 2012-2013 Florian Petran
# set CMAKE_BUILD_TYPE=Debug for debug build
cmake_minimum_required (VERSION 2.8.10)
project (palign)

option(INCLUDE_TESTS "Build all unit tests by default" OFF)
option(INCLUDE_TAGS "Include ctags/cscope generation in default build" OFF)
option(GCOV_COVERAGE "Test coverage with gcov (gcc only)" OFF)
# lolcmake: string options aren't options
set(DESTINATION "/usr/local/bin" CACHE STRING "Directory to install binaries to")
set(STRING_IMPL "ICU" CACHE STRING "String implementation to use (ICU|STD)")

# set module path that find modules from the source tree can be used
# (required for FindICU.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules" )

#####################################################################
# configure macros in source
#####################################################################
# set version number
set(align_VERSION_MAJOR 0)
set(align_VERSION_MINOR 1)
set(align_VERSION_PATCH 0)
set(align_VERSION ${align_VERSION_MAJOR}.${align_VERSION_MINOR}.${align_VERSION_PATCH})
# default value for params
set (align_DEFAULT_MAX_SKIP 1)
set (align_DEFAULT_CLOSENESS 2)
set (align_DEFAULT_DICT_BASE ".")
set (align_DEFAULT_MONOTONY true)
set (dictionary_WORDLENGTH_THRESHOLD 3)
set (dictionary_COGNATE_THRESHOLD 0.7)
# path for tests
set (align_TEST_BASE "${PROJECT_SOURCE_DIR}/test_data")
configure_file (
    "${PROJECT_SOURCE_DIR}/align_config.h.in"
    "${PROJECT_BINARY_DIR}/align_config.h"
    )
include_directories(${PROJECT_BINARY_DIR})

#####################################################################
# find required packages
#####################################################################
find_package(Boost 1.48 COMPONENTS program_options REQUIRED)

if (STRING_IMPL STREQUAL "ICU")
    find_package(ICU 49)
    if (NOT ICU_FOUND)
        set(STRING_IMPL "STD")
        message(SEND_ERROR "ICU Implementation NOT found - defaulting to use std::string")
    else()
        message(STATUS "Using ICU Unicode string implementation")
        add_definitions(-DUSE_ICU_STRING)
        set(STRING_LIBRARY ${ICU_LIBRARIES})
        include_directories(${ICU_INCLUDE_DIRS})
    endif()
elseif(STRING_IMPL STREQUAL "STD")
    message(STATUS "Using std::string implementation")
else()
    message(FATAL_ERROR "string implementation ${STRING_IMPL} not recognized!")
endif()

#####################################################################
# compiler version check
#####################################################################
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -pedantic")
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.6.99")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    elseif (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.5.99")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
        message(FATAL_ERROR "You need at least g++ >= 4.6! Your version is ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
    set(CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -pg" )
    if(GCOV_COVERAGE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ftest-coverage -fprofile-arcs")
    endif()
endif()

if (MSVC)
    message(FATAL_ERROR "Sorry, compilation won't work with MSVC due to it's incomplete C++11 support")
endif()

#####################################################################
# main binaries/libraries
#####################################################################
set(align_SRCS
    align.cpp params.cpp scorers.cpp containers.cpp
    text.cpp dictionary.cpp string_impl.cpp)
set(bisim_SRCS bi-sim.cpp)
add_library(bisim ${bisim_SRCS})
add_library(align ${align_SRCS})
# alignment binary
add_executable(palign main.cpp)
target_link_libraries(align bisim ${STRING_LIBRARY})
target_link_libraries(palign align ${Boost_LIBRARIES} ${STRING_LIBRARY})
# dictionary induction binary
add_executable(mkdict mkdict_main.cpp mkdict.cpp string_impl.cpp)
target_link_libraries(mkdict bisim ${Boost_LIBRARIES} ${STRING_LIBRARY})
install(TARGETS palign mkdict DESTINATION ${DESTINATION})

#####################################################################
# optional stuff to help with development
#####################################################################
# doc generation
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

# tags as target or included in default
find_program(CTAGS ctags)
if(NOT CTAGS STREQUAL "CTAGS-NOTFOUND")
    add_custom_target(ctags
        COMMAND ${CTAGS} --fields=+iaS --extra=+q -R ${CMAKE_SOURCE_DIR}/
        COMMAND cp tags ${CMAKE_SOURCE_DIR}/
    )
else()
    message(STATUS "ctags binary not found, skipping ctags generation.")
    set(CTAGS false)
endif()
find_program(CSCOPE cscope)
if(NOT CSCOPE STREQUAL "CSCOPE-NOTFOUND")
    add_custom_target(cscope
        COMMAND ${CSCOPE} -bR ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.h
        COMMAND cp cscope.out ${CMAKE_SOURCE_DIR}/
    )
else()
    message(STATUS "cscope binary not found, skipping cscope generation.")
    set(CSCOPE false)
endif()
# the assumption here is that if ctags is present, cscope will be too.
# maybe it should be possible to configure it separately, but OTOH the user
# can just install both if he wants one of them. It's for his own good.
if(INCLUDE_TAGS AND CTAGS AND CSCOPE)
    add_custom_target(tags ALL
        COMMAND make ctags
        COMMAND make cscope
    )
elseif(CTAGS AND CSCOPE)
    add_custom_target(tags
        COMMAND make ctags
        COMMAND make cscope
    )
endif()
# google style checker available at
# http://google-styleguide.googlecode.com/svn/trunk/cpplint/cpplint.py
find_program(CPPLINT cpplint.py)
if(NOT CPPLINT STREQUAL "CPPLINT-NOTFOUND")
    add_custom_target(lint
        COMMAND ${CPPLINT} --filter=-readability/streams ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.h
    )
else()
    message(STATUS "C++ style checker not found, skipping lint target.")
endif()

#####################################################################
# testing
#####################################################################
find_package(GTest)
if(GTEST_FOUND)
    set(TESTCASE_LIBS
        align ${GTEST_BOTH_LIBRARIES} ${STRING_LIBRARY} ${Boost_LIBRARIES})
    enable_testing()
    include_directories(${GTEST_INCLUDE_DIRS})

    if(INCLUDE_TESTS)
        add_executable(text_test text_unittest.cpp)
        add_executable(align_test align_unittest.cpp)
        add_executable(bisim_test bisim_test.cpp)
    else()
        set(${CMAKE_TEST_COMMAND} "cmake -V")
        add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
        add_executable(text_test EXCLUDE_FROM_ALL text_unittest.cpp)
        add_executable(align_test EXCLUDE_FROM_ALL align_unittest.cpp)
        add_executable(bisim_test EXCLUDE_FROM_ALL bisim_test.cpp)
        add_dependencies(check align_test text_test bisim_test)
    endif()
    target_link_libraries(text_test ${TESTCASE_LIBS})
    target_link_libraries(align_test ${TESTCASE_LIBS})
    target_link_libraries(bisim_test ${GTEST_BOTH_LIBRARIES} ${STRING_LIBRARY} bisim)

    add_test(text_test text_test)
    add_test(align_test align_test)
    add_test(bisim_test bisim_test)
endif()

