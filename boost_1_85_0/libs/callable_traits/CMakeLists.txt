if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)

# Generated by `boostdep --cmake callable_traits`
# Copyright 2020, 2021 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.8...3.20)

project(boost_callable_traits VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX)

add_library(boost_callable_traits INTERFACE)
add_library(Boost::callable_traits ALIAS boost_callable_traits)

target_include_directories(boost_callable_traits INTERFACE include)

target_compile_features(boost_callable_traits INTERFACE cxx_std_11)

else()

# Copyright Louis Dionne 2015
# Modified Work Copyright Barrett Adair 2015-2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.0)
project(boost_callable_traits CXX)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
enable_testing()

set (CMAKE_CXX_STANDARD ${boost_callable_traits_CXX_STD})

# Setting up CMake options and compiler flags (more flags can be set on a per-target basis or in subdirectories)

include(CheckCXXCompilerFlag)
macro(boost_callable_traits_append_flag testname flag)
    check_cxx_compiler_flag(${flag} ${testname})
    if (${testname})
        add_compile_options(${flag})
    endif()
endmacro()

if(NOT MSVC OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")

    # enable all warnings and treat them all as errors
    boost_callable_traits_append_flag(boost_callable_traits_HAS_WERROR -Werror)
    boost_callable_traits_append_flag(boost_callable_traits_HAS_WX -WX)
    boost_callable_traits_append_flag(boost_callable_traits_HAS_W                          -W)
    boost_callable_traits_append_flag(boost_callable_traits_HAS_WALL                       -Wall)
    boost_callable_traits_append_flag(boost_callable_traits_HAS_WEXTRA                     -Wextra)
endif()

if(MSVC)

    # MSVC/Clang-cl builds need -Qunused-arguments
    boost_callable_traits_append_flag(boost_callable_traits_HAS_QUNUSED_ARGUMENTS          -Qunused-arguments)
else()

    # for better template error debugging
    boost_callable_traits_append_flag(boost_callable_traits_HAS_FTEMPLATE_BACKTRACE_LIMIT  -ftemplate-backtrace-limit=0)
    
    # enforce strict standards compliance
    boost_callable_traits_append_flag(boost_callable_traits_HAS_PEDANTIC                   -pedantic)
    
    # use the most recent C++ standard available
    boost_callable_traits_append_flag(boost_callable_traits_HAS_STDCXX0x                   -std=c++0x)
    boost_callable_traits_append_flag(boost_callable_traits_HAS_STDCXX1y                   -std=c++1y)
    boost_callable_traits_append_flag(boost_callable_traits_HAS_STDCXX1z                   -std=c++1z)
    boost_callable_traits_append_flag(boost_callable_traits_HAS_STDCXX17                   -std=c++17)
    boost_callable_traits_append_flag(boost_callable_traits_HAS_STDCXX2a                   -std=c++2a)
    boost_callable_traits_append_flag(boost_callable_traits_HAS_STDCXX20                   -std=c++20)
    boost_callable_traits_append_flag(boost_callable_traits_HAS_STDCXX2b                   -std=c++2b)
endif()

# transactional memory - currently only available in GCC 6 and later
if(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    boost_callable_traits_append_flag(boost_callable_traits_HAS_FGNU_TM                    -fgnu-tm)
endif()

add_library(boost_callable_traits INTERFACE)
set_property(TARGET boost_callable_traits PROPERTY EXPORT_NAME callable_traits)
add_library(Boost::callable_traits ALIAS boost_callable_traits)

target_include_directories(boost_callable_traits INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    $<INSTALL_INTERFACE:include>)

#
#find_package(Doxygen)
##find_package(Meta)
#find_package(PythonInterp 2.7)
#find_package(Ruby 2.1)

##############################################################################
# boost_callable_traits_target_name_for(<output variable> <source file> [ext])
# Returns the target name associated to a source file. If the path of the
# source file relative from the root of boost_callable_traits is `path/to/source/file.ext`,
# the target name associated to it will be `path.to.source.file`.
#
# The extension of the file should be specified as a last argument. If no
# extension is specified, the `.cpp` extension is assumed.
##############################################################################

function(boost_callable_traits_target_name_for out file)
    if (NOT ARGV2)
        set(_extension ".cpp")
    else()
        set(_extension "${ARGV2}")
    endif()

    file(RELATIVE_PATH _relative ${boost_callable_traits_SOURCE_DIR} ${file})
    string(REPLACE "${_extension}" "" _name ${_relative})
    string(REGEX REPLACE "/" "." _name ${_name})
    set(${out} "${_name}" PARENT_SCOPE)
endfunction()

##############################################################################
# boost_callable_traits_add_test(<name> <command> [<arg>...])
# Creates a test called `name`, which runs the given `command` with the given args.
##############################################################################

function(boost_callable_traits_add_test name)
    if (boost_callable_traits_ENABLE_MEMCHECK)
        add_test(${name} ${Valgrind_EXECUTABLE} --leak-check=full --error-exitcode=1 ${ARGN})
    else()
        add_test(${name} ${ARGN})
    endif()
endfunction()

##############################################################################
# Setup the `check` target to build and then run all the tests and examples.
##############################################################################

add_custom_target(callable_traits_check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Build and then run all the tests and examples.")
if (NOT TARGET check)
    add_custom_target(check DEPENDS callable_traits_check)
else()
    add_dependencies(check callable_traits_check)
endif()

add_subdirectory(example)
add_subdirectory(test)

##############################################################################
# Setup the 'install' target and the package config file.
##############################################################################
install(TARGETS boost_callable_traits     EXPORT CallableTraitsConfig)
install(EXPORT CallableTraitsConfig DESTINATION lib/cmake/CallableTraits)
install(DIRECTORY include/boost     DESTINATION include FILES_MATCHING PATTERN "*.hpp")

endif()
