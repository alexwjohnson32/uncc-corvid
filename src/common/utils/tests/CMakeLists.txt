# Stopwatch testing
set(STOPWATCH_TEST stopwatch_test)

add_executable(${STOPWATCH_TEST} stopwatch_tests.cpp)
target_link_libraries(${STOPWATCH_TEST} PRIVATE ${CORVID_UTILS_LIB} ${CORVID_TESTING_LIB})
add_test(NAME StopwatchTest COMMAND ${STOPWATCH_TEST})

# Json Templates Testing
set(JSON_TEMPLATES_TEST json_templates_test)

add_executable(${JSON_TEMPLATES_TEST} json_templates_tests.cpp)
target_link_libraries(${JSON_TEMPLATES_TEST} PRIVATE ${CORVID_UTILS_LIB} ${CORVID_TESTING_LIB})
add_test(NAME JsonTemplatesTest COMMAND ${JSON_TEMPLATES_TEST})

# Websocket Client Testing
set(WEBSOCKET_CLIENT_TEST websocket_client_test)

add_executable(${WEBSOCKET_CLIENT_TEST} websocket_client_tests.cpp mock_websocket_server.hpp)
target_link_libraries(${WEBSOCKET_CLIENT_TEST} PRIVATE ${CORVID_UTILS_LIB} ${CORVID_TESTING_LIB})
add_test(NAME WebSocketClientTest COMMAND ${WEBSOCKET_CLIENT_TEST})