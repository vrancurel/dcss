add_custom_target(${JSON_RPC_STUB}
  DEPENDS ${GENERATE_DIR}/gethclient.h
  DEPENDS ${GENERATE_DIR}/kadclient.h
)

add_custom_command(
  OUTPUT  ${GENERATE_DIR}/gethclient.h
  COMMAND jsonrpcstub ${CMAKE_CURRENT_SOURCE_DIR}/geth.json --cpp-client=GethClient --cpp-client-file=${GENERATE_DIR}/gethclient.h
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/geth.json
  COMMENT "Generating JSON-RPC client for Ethereum communication"
)

add_custom_command(
  OUTPUT  ${GENERATE_DIR}/kadclient.h
  COMMAND jsonrpcstub ${CMAKE_CURRENT_SOURCE_DIR}/kad.json --cpp-client=KadClient --cpp-client-file=${GENERATE_DIR}/kadclient.h
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kad.json
  COMMENT "Generating JSON-RPC client for Node-to-Node communication"
)
