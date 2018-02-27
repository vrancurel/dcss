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
add_custom_target(${JSON_RPC_STUB}
  DEPENDS ${GENERATE_DIR}/gethclient.h
  DEPENDS ${GENERATE_DIR}/nodeclient.h
)

add_custom_command(
  OUTPUT  ${GENERATE_DIR}/gethclient.h
  COMMAND jsonrpcstub ${CMAKE_CURRENT_SOURCE_DIR}/geth.json --cpp-client=GethClient --cpp-client-file=${GENERATE_DIR}/gethclient.h
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/geth.json
  COMMENT "Generating JSON-RPC client for Ethereum communication"
)

add_custom_command(
  OUTPUT  ${GENERATE_DIR}/nodeclient.h
  COMMAND jsonrpcstub ${CMAKE_CURRENT_SOURCE_DIR}/node.json --cpp-client=NodeClient --cpp-client-file=${GENERATE_DIR}/nodeclient.h
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/node.json
  COMMENT "Generating JSON-RPC client for Node-to-Node communication"
)
