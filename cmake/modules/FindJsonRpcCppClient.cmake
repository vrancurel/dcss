# Copyright 2017-2018 the DCSS authors
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
