cmake_minimum_required(VERSION 3.5)

######################
# Project informations
######################

project(quadiron CXX)

enable_testing()

set(CMAKE_CXX_STANDARD          11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE VERSION_REV
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(VERSION_REV)
  set(VERSION_REV "-${VERSION_REV}")
endif()

set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_REV})
set(PACKAGE ${CMAKE_PROJECT_NAME})

################
# Useful defines
################

# Paths
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(GENERATE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin/${CMAKE_BUILD_TYPE})

# Targets
set(JSON_RPC_STUB jsonrpcstup)
set(OBJECT_LIB ${CMAKE_PROJECT_NAME}_object)
set(SHARED_LIB ${CMAKE_PROJECT_NAME}_shared)
set(STATIC_LIB ${CMAKE_PROJECT_NAME}_static)

file(MAKE_DIRECTORY ${GENERATE_DIR})
# Use SYSTEM so that our strict compilers settings are not applied to generated
# code.
include_directories(SYSTEM
    ${GENERATE_DIR}
)

####################
# Default build type
####################

# From https://blog.kitware.com/cmake-and-the-default-build-type/

# Set a default build type if none was specified.
set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(default_build_type "Debug")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}"
    CACHE
    STRING
    "Choose the type of build."
    FORCE
  )
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
  )
endif()

################
# Compiler flags
################

include(CheckCXXCompilerFlag)

set(COMMON_CXX_FLAGS
  -pipe
  -Wall -Wextra -Wdeprecated -Wpedantic
  -Wdocumentation -Wdocumentation-pedantic
  -Wconversion -Wno-sign-conversion
  -Wunused-exception-parameter -Wunused-macros -Wunused-member-function -Wunused-template
  -Wgcc-compat -Wgnu
  -Warray-bounds-pointer-arithmetic
  -Wassign-enum
  -Wbad-function-cast -Wcast-qual -Wcast-align -Wold-style-cast -Wuseless-cast
  -Wconditional-uninitialized
  -Wduplicate-method-arg -Wduplicate-method-match
  -Wfloat-equal -Wdouble-promotion
  -Wformat-pedantic
  -Winit-self
  -Wlogical-op
  -Wmissing-noreturn -Wmissing-prototypes -Wmissing-include-dirs -Wmissing-declarations
  -Wnested-anon-types
  -Wnon-virtual-dtor
  -Woverlength-strings -Wwrite-strings
  -Woverloaded-virtual
  -Wredundant-decls
  -Wshadow
  -Wshift-sign-overflow
  -Wstring-compare -Wstring-conversion -Wstring-plus-char
  -Wstrict-null-sentinel
  -Wsometimes-uninitialized
  -Wswitch-enum
  -Wunreachable-code-aggressive
)

# Manually add -Werror, for some reasons I can't make it works in the foreach…
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-Werror)
endif()

foreach(flag ${COMMON_CXX_FLAGS})
  check_cxx_compiler_flag(${flag} has_flag_${flag})
  if(has_flag_${flag})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
  endif(has_flag_${flag})
endforeach(flag)

# Debug flags
set(DEBUG_CXX_FLAGS
  -g3 -ggdb3 -fno-limit-debug-info
  -ftrapv
)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS}")
foreach(flag ${DEBUG_CXX_FLAGS})
  check_cxx_compiler_flag(${flag} has_flag_${flag})
  if(has_flag_${flag})
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${flag}")
  endif(has_flag_${flag})
endforeach(flag)

# Release flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS}")
foreach(flag -O3 -flto -DNDEBUG)
  check_cxx_compiler_flag(${flag} has_flag_${flag})
  if(has_flag_${flag})
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${flag}")
  endif(has_flag_${flag})
endforeach(flag)

################
# QuadIron build
################

# JSON-RPC stubs
add_subdirectory(jsonrpc_spec)
# QuadIron
add_subdirectory(src)
# Tests
add_subdirectory(test)
# Documentation
add_subdirectory(documentation)

#####################
# Formatting & Linter
#####################

set(ALL_SRCS
  ${LIB_SRC}
  ${TEST_SRC}
  ${SOURCE_DIR}/main.cpp
  ${SOURCE_DIR}/cmds.h
  ${SOURCE_DIR}/bignum.h
  ${SOURCE_DIR}/bit_map.h
  ${SOURCE_DIR}/kad_conf.h
  ${SOURCE_DIR}/kad_file.h
  ${SOURCE_DIR}/kad_network.h
  ${SOURCE_DIR}/kad_node.h
  ${SOURCE_DIR}/kad_routable.h
  ${SOURCE_DIR}/kadsim.h
  ${SOURCE_DIR}/shell.h
  ${SOURCE_DIR}/utils.h
)

add_custom_target(format
  COMMAND clang-format -i ${ALL_SRCS}
  COMMENT "fix code formatting"
)
add_dependencies(format ${PROJECT_NAME})

# We still need these ones for the headers files (not in compile_commands.json).
set(CLANG_LINT_OPTS
  -x c++ --std=c++11
  -isystem ${GENERATE_DIR}
  ${GTEST_INCLUDE_PATHS}
)
set(CLANG_LINTS_LIST
  android-*
  boost-*
  bugprone-*
  cert-*
  clang-analyzer-*
  google-build-explicit-make-pair
  google-default-arguments
  google-explicit-constructor
  google-readability-casting
  google-runtime-member-string-references
  google-runtime-int
  hicpp-avoid-goto
  hicpp-exception-baseclass
  hicpp-member-init
  hicpp-no-array-decay
  hicpp-no-assembler
  hicpp-no-malloc
  hicpp-signed-bitwise
  hicpp-special-member-functions
  hicpp-vararg
  misc-*
  modernize-*
  mpi-*
  performance-*
  readability-*
)
string(REPLACE ";" "," CLANG_LINTS "${CLANG_LINTS_LIST}")

add_custom_target(lint
  COMMAND clang-tidy ${ALL_SRCS} -checks=${CLANG_LINTS} -p ${CMAKE_BINARY_DIR} -- ${CLANG_LINT_OPTS}
  COMMENT "run the linter"
)
add_dependencies(lint ${PROJECT_NAME})

add_custom_target(fix-lint
  COMMAND clang-tidy -fix ${ALL_SRCS} -checks=${CLANG_LINTS} -p ${CMAKE_BINARY_DIR} -- ${CLANG_LINT_OPTS}
  COMMENT "run the linter and apply proposed fixes"
)
add_dependencies(fix-lint ${PROJECT_NAME})

####################
# Targets for the CI
####################

# clang-tidy returns 0, even if there are lint errors, so we have to be hackish.
# Should be no longer needed when https://reviews.llvm.org/D39105 is merged…
set(LINT_ERROR_PATTERN "'(error|warning):'")
add_custom_target(check-lint
  COMMAND test `clang-tidy ${ALL_SRCS} -checks=${CLANG_LINTS} -p ${CMAKE_BINARY_DIR} -- ${CLANG_LINT_OPTS} | grep -Pc ${LINT_ERROR_PATTERN}` -eq 0
  COMMENT "check for lint errors"
)
add_dependencies(check-lint ${PROJECT_NAME})

add_custom_target(check-format
  COMMAND test `clang-format -output-replacements-xml ${ALL_SRCS} | grep -c offset` -eq 0
  COMMENT "check code formatting"
)
add_dependencies(check-format ${PROJECT_NAME})
