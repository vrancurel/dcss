cmake_minimum_required(VERSION 3.5)

######################
# Project informations
######################

project(quadiron CXX)
set(CMAKE_CXX_STANDARD          11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

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

include_directories("src")

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

set(EXECUTABLE_NAME ${CMAKE_PROJECT_NAME})

# Dependencies.
find_package(OpenSSL      REQUIRED)
find_package(Readline     REQUIRED)
find_package(CURL         REQUIRED)
find_package(Jsoncpp      REQUIRED)
find_package(json_rpc_cpp REQUIRED)

add_definitions(-DHAVE_READLINE)

# Source files.
set(SRCS
  ${SOURCE_DIR}/bit_map.cpp
  ${SOURCE_DIR}/cmds.cpp
  ${SOURCE_DIR}/kad_file.cpp
  ${SOURCE_DIR}/kad_network.cpp
  ${SOURCE_DIR}/kad_node.cpp
  ${SOURCE_DIR}/kad_routable.cpp
  ${SOURCE_DIR}/main.cpp
  ${SOURCE_DIR}/shell.cpp
)

configure_file(${SOURCE_DIR}/config.in ${GENERATE_DIR}/config.h @ONLY)

add_executable(${EXECUTABLE_NAME}
  ${SRCS}
)

# Link librairies.
target_link_libraries(${EXECUTABLE_NAME}
  ${OPENSSL_CRYPTO_LIBRARY}
  ${Readline_LIBRARY}
  ${JSONCPP_LIBRARY}
  ${CURL_LIBRARIES}
  ${JSON_RPC_CPP_CLIENT_LIBRARIES}
)

# JSON-RPC stubs
add_subdirectory(jsonrpc_spec)
