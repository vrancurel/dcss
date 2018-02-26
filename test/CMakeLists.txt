# Copyright 2017-2018 the QuadIron authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

#########################################################
# Google Test
# Adapted from http://www.kaizou.org/2014/11/gtest-cmake/
#########################################################

find_package(Threads REQUIRED)

# Download and install GoogleTest
#
# To know WHY we have to download and build it ourself, see the FAQ:
# - https://github.com/google/googletest/blob/master/googletest/docs/FAQ.md
include(ExternalProject)

ExternalProject_Add(gtest
  PREFIX          ${CMAKE_CURRENT_BINARY_DIR}/gtest
  GIT_REPOSITORY  https://github.com/google/googletest.git
  GIT_TAG         release-1.8.0
  INSTALL_COMMAND ""
)

# Get paths of the installed GoogleTest.
ExternalProject_Get_Property(gtest source_dir binary_dir)
set(GTEST_SOURCE_DIR ${source_dir})

# GoogleTest target (to be used as a dependency by our test driver).
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)
set_target_properties(libgtest PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)

# GoogleMock target (to be used as a dependency by our test driver).
add_library(libgmock IMPORTED STATIC GLOBAL)
add_dependencies(libgmock gtest)
set_target_properties(libgmock PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)

###########################################################################
# GTEST_ADD_TESTS
# From https://github.com/Kitware/CMake/blob/v3.5.0/Modules/FindGTest.cmake
###########################################################################

# With a modern CMake, we could use `GoogleTest` directly.
# By using a pre-built GoogleTest we could use `find_package(GTest)`.
#
# Since we are not in these case, we will defines the function ourself.
function(GTEST_ADD_TESTS executable extra_args)
  if(NOT ARGN)
    message(FATAL_ERROR "Missing ARGN: Read the documentation for GTEST_ADD_TESTS")
  endif()
  if(ARGN STREQUAL "AUTO")
    # obtain sources used for building that executable
    get_property(ARGN TARGET ${executable} PROPERTY SOURCES)
  endif()
  set(gtest_case_name_regex ".*\\( *([A-Za-z_0-9]+) *, *([A-Za-z_0-9]+) *\\).*")
  set(gtest_test_type_regex "(TYPED_TEST|TEST_?[FP]?)")
  foreach(source ${ARGN})
    file(READ "${source}" contents)
    string(REGEX MATCHALL "${gtest_test_type_regex} *\\(([A-Za-z_0-9 ,]+)\\)" found_tests ${contents})
    foreach(hit ${found_tests})
      string(REGEX MATCH "${gtest_test_type_regex}" test_type ${hit})

      # Parameterized tests have a different signature for the filter
      if("x${test_type}" STREQUAL "xTEST_P")
        string(REGEX REPLACE ${gtest_case_name_regex}  "*/\\1.\\2/*" test_name ${hit})
      elseif("x${test_type}" STREQUAL "xTEST_F" OR "x${test_type}" STREQUAL "xTEST")
        string(REGEX REPLACE ${gtest_case_name_regex} "\\1.\\2" test_name ${hit})
      elseif("x${test_type}" STREQUAL "xTYPED_TEST")
        string(REGEX REPLACE ${gtest_case_name_regex} "\\1/*.\\2" test_name ${hit})
      else()
        message(WARNING "Could not parse GTest ${hit} for adding to CTest.")
        continue()
      endif()
      add_test(NAME ${test_name} COMMAND ${executable} --gtest_filter=${test_name} ${extra_args})
    endforeach()
  endforeach()
endfunction()

#############
# Build tests
#############

# Source files.
set(TEST_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/bit_map.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp

  CACHE
  INTERNAL
  ""
  FORCE
)

set(TEST_DRIVER unit_tests)
add_executable(${TEST_DRIVER}
  ${TEST_SRC}
)
add_coverage(${TEST_DRIVER})

target_link_libraries(${TEST_DRIVER}
  libgtest
  ${STATIC_LIB}
)
# Use SYSTEM so that our strict compilers settings are not applied on this code.
target_include_directories(${TEST_DRIVER} SYSTEM
    PUBLIC "${GTEST_SOURCE_DIR}/googletest/include"
    PUBLIC "${source_dir}/googlemock/include"
)

GTEST_ADD_TESTS(${TEST_DRIVER} "" ${TEST_SRC})

# Be sure to recompile the test driver if needed before running the test suite.
add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND}
  DEPENDS ${TEST_DRIVER}
)
