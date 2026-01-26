# Stopwatch testing
set(STOPWATCH_TEST stopwatch_test)
message("STOPWATCH_TEST: ${STOPWATCH_TEST}")

add_executable(${STOPWATCH_TEST} stopwatch_tests.cpp)
target_link_libraries(${STOPWATCH_TEST} PRIVATE ${CORVID_UTILS_LIB} ${CORVID_TESTING_LIB})
add_test(NAME StopwatchTest COMMAND ${STOPWATCH_TEST})

message("Stopwatch tests built!")

# Json Templates Testing
set(JSON_TEMPLATES_TEST json_templates_test)
message("JSON_TEMPLATES_TEST: ${JSON_TEMPLATES_TEST}")

add_executable(${JSON_TEMPLATES_TEST} json_templates_tests.cpp)
target_link_libraries(${JSON_TEMPLATES_TEST} PRIVATE ${CORVID_UTILS_LIB} ${CORVID_TESTING_LIB})
add_test(NAME JsonTemplatesTest COMMAND ${JSON_TEMPLATES_TEST})

message("Json Templates Tests built!")