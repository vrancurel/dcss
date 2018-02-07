# Source files.
set(LIB_SRC
  ${SOURCE_DIR}/bit_map.cpp
  ${SOURCE_DIR}/cmds.cpp
  ${SOURCE_DIR}/kad_conf.cpp
  ${SOURCE_DIR}/kad_file.cpp
  ${SOURCE_DIR}/kad_network.cpp
  ${SOURCE_DIR}/kad_node.cpp
  ${SOURCE_DIR}/kad_routable.cpp
  ${SOURCE_DIR}/io.cpp
  ${SOURCE_DIR}/shell.cpp

  CACHE
  INTERNAL
  ""
  FORCE
)

# Generate config.h (with compile-time information).
configure_file(${SOURCE_DIR}/config.in ${GENERATE_DIR}/config.h @ONLY)

###########
# Libraries
###########

# Build an Object Library (can be reused for both static and dynamic libs).
add_library(${OBJECT_LIB} OBJECT ${LIB_SRC})
add_coverage(${OBJECT_LIB})
set_property(TARGET ${OBJECT_LIB} PROPERTY POSITION_INDEPENDENT_CODE 1)
add_dependencies(${OBJECT_LIB}
  ${JSON_RPC_STUB}
)

# Dynamic library.
add_library(${SHARED_LIB} SHARED $<TARGET_OBJECTS:${OBJECT_LIB}>)
set_target_properties(${SHARED_LIB} PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME})
# Static library.
add_library(${STATIC_LIB} STATIC $<TARGET_OBJECTS:${OBJECT_LIB}>)
set_target_properties(${STATIC_LIB} PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME})

############
# Executable
############

set(EXECUTABLE_NAME ${CMAKE_PROJECT_NAME})

# Dependencies.
find_package(OpenSSL      REQUIRED)
find_package(Readline     REQUIRED)
find_package(CURL         REQUIRED)
find_package(Jsoncpp      REQUIRED)
find_package(json_rpc_cpp REQUIRED)
find_package(ntl          REQUIRED)

add_definitions(-DHAVE_READLINE)

add_executable(${EXECUTABLE_NAME}
  ${SOURCE_DIR}/main.cpp
)
add_coverage(${EXECUTABLE_NAME})

# Link librairies.
target_link_libraries(${EXECUTABLE_NAME}
  ${STATIC_LIB}
  ${OPENSSL_CRYPTO_LIBRARY}
  ${Readline_LIBRARY}
  ${JSONCPP_LIBRARY}
  ${CURL_LIBRARIES}
  ${JSON_RPC_CPP_CLIENT_LIBRARIES}
  ${NTL_LIBRARY}
)

