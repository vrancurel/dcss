# - Try to find libjson-rpc-cpp for client side
#
# Once done this will define
#  JsonRpcCppClient_FOUND         - System has libjson-rpc-cpp (client).
#  JsonRpcCppClient_INCLUDE_DIRS  - The libjson-rpc-cpp (client) include directories.
#  JsonRpcCppClient_LIBRARIES     - The libraries needed to use libjson-rpc-cpp (client).

include(LibFindMacros)

# Dependencies.
libfind_package(JsonRpcCppClient JsonRpcCppCommon)
libfind_package(JsonRpcCppClient CURL)
libfind_package(JsonRpcCppClient Hiredis)

# Use pkg-config to get hints about paths.
libfind_pkg_check_modules(JsonRpcCppClient_PKGCONF jsonrpccpp-client)

# Include directory.
find_path(JsonRpcCppClient_INCLUDE_DIR
  NAMES jsonrpccpp/client.h
  PATHS ${JsonRpcCppClient_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself.
find_library(JsonRpcCppClient_LIBRARY
  NAMES jsonrpccpp-client
  PATHS ${JsonRpcCppClient_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir/libraries variables and let libfind_process do the rest.
libfind_process(JsonRpcCppClient)
