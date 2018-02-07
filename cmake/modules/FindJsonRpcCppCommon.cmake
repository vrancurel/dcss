# - Try to find libjson-rpc-cpp (common)
#
# Once done this will define
#  JsonRpcCppCommon_FOUND         - System has libjson-rpc-cpp (common)
#  JsonRpcCppCommon_INCLUDE_DIRS  - The libjson-rpc-cpp (common) include directories.
#  JsonRpcCppCommon_LIBRARIES     - The libraries needed to use libjson-rpc-cpp (common).

include(LibFindMacros)

# Dependencies.
libfind_package(JsonRpcCppCommon JsonCpp)

# Use pkg-config to get hints about paths.
libfind_pkg_check_modules(JsonRpcCppCommon_PKGCONF jsonrpccpp-common)

# Include directory.
find_path(JsonRpcCppCommon_INCLUDE_DIR
  NAMES jsonrpccpp/server.h
  PATHS ${JsonRpcCppCommon_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself.
find_library(JsonRpcCppCommon_LIBRARY
  NAMES jsonrpccpp-common jsonrpccpp-client jsonrpccpp-server
  PATHS ${JsonRpcCppCommon_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir/libraries variables and let libfind_process do the rest.
libfind_process(JsonRpcCppCommon)
