cmake_minimum_required (VERSION 2.8)
project (align)
option(INCLUDE_TESTS "Build all unit tests by default")
option(INCLUDE_TAGS "Include ctags/cscope generation in default build")

# set module path that find modules from the source tree can be used
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules" )

# set version number
set (align_VERSION_MAJOR 0)
set (align_VERSION_MINOR 1)
# this crap is used for testing
set (align_DICT_BASE "${PROJECT_SOURCE_DIR}/test_data/dict/")
set (align_FILE_BASE "${PROJECT_SOURCE_DIR}/test_data")
set (align_TEST_E "21_Ba.txt")
set (align_TEST_F "65_M21.txt")
# default value for params
set (align_DEFAULT_MAX_SKIP 1)
set (align_DEFAULT_CLOSENESS 2)
# this is for stuff that can be set by cmake
configure_file (
    "${PROJECT_SOURCE_DIR}/align_config.h.in"
    "${PROJECT_BINARY_DIR}/align_config.h"
    )

find_package( Boost 1.48 COMPONENTS program_options REQUIRED )
find_package( ICU 49 REQUIRED )
if (CMAKE_COMPILER_IS_GNUCXX)
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -pedantic -std=c++0x" )
    set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -pg" )
endif()
if (MSVC)
    list( APPEND CMAKE_CXX_FLAGS_DEBUG " /W4" )
    list( REMOVE_DUPLICATES CMAKE_CXX_FLAGS_DEBUG )
endif()

# checking for C++11 extensions flag availability - required for lots of stuff
# currently only works for gcc, needs to be extended to other platforms/compilers
# XXX
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag( -std=c++0x SUPPORTS_CXX_0x )
message( STATUS "Checking if compiler supports C++0x extensions..." )
if(NOT SUPPORTS_CXX_0x)
    message( FATAL_ERROR "C++0x extensions not supported by your compiler!")
endif()

set(
    align_SRCS
    align.cpp params.cpp scorers.cpp containers.cpp
    text.cpp dictionary.cpp string_impl.cpp
)
set(
    bisim_SRCS
    bi-sim.cpp
)
include_directories( ${ICU_INCLUDE_DIRS} ${PROJECT_BINARY_DIR} )
add_library( bisim ${bisim_SRCS} )
add_library( align ${align_SRCS} )
add_executable( palign main.cpp )
target_link_libraries( align bisim )
target_link_libraries( palign
    align
    ${Boost_LIBRARIES}
    ${ICU_LIBRARIES}
    )
install (TARGETS palign DESTINATION /home/anselm/bin)

# optional stuff
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

find_program(CTAGS ctags)
find_program(CSCOPE cscope)
add_custom_target(ctags
    COMMAND ${CTAGS} --fields=+iaS --extra=+q -R ${CMAKE_SOURCE_DIR}/
    COMMAND cp tags ${CMAKE_SOURCE_DIR}/
)
add_custom_target(cscope
    COMMAND ${CSCOPE} -bR ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.h
    COMMAND cp cscope.out ${CMAKE_SOURCE_DIR}/
)
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

find_package( GTest )
if(GTEST_FOUND)
    set (TESTCASE_LIBS align ${GTEST_BOTH_LIBRARIES} ${ICU_LIBRARIES})
    enable_testing()
    include_directories(${GTEST_INCLUDE_DIRS})

    if(INCLUDE_TESTS)
        add_executable( text_test
            text_unittest.cpp )
        add_executable( align_test
            align_unittest.cpp )
    else()
        set(${CMAKE_TEST_COMMAND} "cmake -V")
        add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
        add_executable( text_test EXCLUDE_FROM_ALL text_unittest.cpp )
        add_executable( align_test EXCLUDE_FROM_ALL align_unittest.cpp )
        add_dependencies(check align_test text_test)
    endif()
    target_link_libraries( text_test ${TESTCASE_LIBS} )
    add_test(text_test text_test)

    add_test(align_test align_test)
    target_link_libraries( align_test ${TESTCASE_LIBS} )
endif()

