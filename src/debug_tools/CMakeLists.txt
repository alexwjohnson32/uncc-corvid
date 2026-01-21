add_executable(testing_client testing_websocket_client.cpp)
target_link_libraries(testing_client ${CORVID_UTILS_LIB})

add_executable(testing_server testing_websocket_server.cpp)
target_link_libraries(testing_server ${CORVID_UTILS_LIB})