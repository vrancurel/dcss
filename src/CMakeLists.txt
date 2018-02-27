# Source files.
set(LIB_SRC
  ${SOURCE_DIR}/bit_map.cpp
  ${SOURCE_DIR}/cmds.cpp
  ${SOURCE_DIR}/kad_conf.cpp
  ${SOURCE_DIR}/kad_file.cpp
  ${SOURCE_DIR}/kad_network.cpp
  ${SOURCE_DIR}/kad_node.cpp
  ${SOURCE_DIR}/kad_routable.cpp
  ${SOURCE_DIR}/io_client.cpp
  ${SOURCE_DIR}/io_server.cpp
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

# Dependencies.
find_package(OpenSSL          REQUIRED)
find_package(Readline         REQUIRED)
find_package(JsonRpcCppClient REQUIRED)
find_package(NTTEC            REQUIRED)

get_property(NTTEC_INCLUDE_DIRS
  TARGET   NTTEC::static
  PROPERTY INTERFACE_INCLUDE_DIRECTORIES
)

set(OBJECT_SYS_INCLUDES
  ${GENERATE_DIR}
  ${OPENSSL_INCLUDE_DIR}
  ${JsonRpcCppClient_INCLUDE_DIRS}
  ${Readline_INCLUDE_DIRS}
  ${NTTEC_INCLUDE_DIRS}
)

set(OBJECT_INCLUDES
  ${SOURCE_DIR}
)

# Build an Object Library (can be reused for both static and dynamic libs).
add_library(${OBJECT_LIB} OBJECT ${LIB_SRC})
add_coverage(${OBJECT_LIB})
set_property(TARGET ${OBJECT_LIB} PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(${OBJECT_LIB} SYSTEM PUBLIC ${OBJECT_SYS_INCLUDES})
target_include_directories(${OBJECT_LIB}        PUBLIC ${OBJECT_INCLUDES})
target_compile_definitions(${OBJECT_LIB}
  PUBLIC HAVE_READLINE=1
)
add_dependencies(${OBJECT_LIB}
  ${JSON_RPC_STUB}
)

# Dynamic library.
add_library(${SHARED_LIB} SHARED $<TARGET_OBJECTS:${OBJECT_LIB}>)
# Static library.
add_library(${STATIC_LIB} STATIC $<TARGET_OBJECTS:${OBJECT_LIB}>)

foreach(lib ${SHARED_LIB} ${STATIC_LIB})
  set_target_properties(${lib} PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME})
  target_include_directories(${lib} SYSTEM PUBLIC ${OBJECT_SYS_INCLUDES})
  target_include_directories(${lib}        PUBLIC ${OBJECT_INCLUDES})
  target_compile_definitions(${lib}
    PUBLIC $<TARGET_PROPERTY:${OBJECT_LIB},COMPILE_DEFINITIONS>
  )

  target_link_libraries(${lib}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${Readline_LIBRARIES}
    ${JsonRpcCppClient_LIBRARIES}
    NTTEC::static
  )
endforeach()

############
# Executable
############

set(EXECUTABLE_NAME ${CMAKE_PROJECT_NAME})

add_executable(${EXECUTABLE_NAME}
  ${SOURCE_DIR}/main.cpp
)
add_coverage(${EXECUTABLE_NAME})

# Link librairies.
target_link_libraries(${EXECUTABLE_NAME}
  ${STATIC_LIB}
)
