set(CLIENT_NAME testing_client)
set(SERVER_NAME testing_server)
set(CORVID_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/debug_tools)

add_executable(${CLIENT_NAME} testing_websocket_client.cpp)
target_link_libraries(${CLIENT_NAME} PRIVATE ${CORVID_UTILS_LIB})

add_executable(${SERVER_NAME} testing_websocket_server.cpp)
target_link_libraries(${SERVER_NAME} PRIVATE ${CORVID_UTILS_LIB})

install(TARGETS ${CLIENT_NAME} ${SERVER_NAME} DESTINATION ${CORVID_INSTALL_DIR})