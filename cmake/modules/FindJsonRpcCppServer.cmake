# - Try to find libjson-rpc-cpp for server side
#
# Once done this will define
#  JsonRpcCppServer_FOUND         - System has libjson-rpc-cpp (server).
#  JsonRpcCppServer_INCLUDE_DIRS  - The libjson-rpc-cpp (server) include directories.
#  JsonRpcCppServer_LIBRARIES     - The libraries needed to use libjson-rpc-cpp (server).

include(LibFindMacros)

# Dependencies.
libfind_package(JsonRpcCppServer JsonRpcCppCommon)
libfind_package(JsonRpcCpp MHD)
libfind_package(JsonRpcCppServer Hiredis)
libfind_package(JsonRpcCpp Threads)

# Use pkg-config to get hints about paths.
libfind_pkg_check_modules(JsonRpcCppServer_PKGCONF jsonrpccpp-server)

# Include directory.
find_path(JsonRpcCppServer_INCLUDE_DIR
  NAMES jsonrpccpp/server.h
  PATHS ${JsonRpcCppServer_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself.
find_library(JsonRpcCppServer_LIBRARY
  NAMES jsonrpccpp-server
  PATHS ${JsonRpcCppServer_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir/libraries variables and let libfind_process do the rest.
libfind_process(JsonRpcCppServer)
