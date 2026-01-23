set(STOPWATCH_TEST stopwatch_test)

message("STOPWATCH_TEST: ${STOPWATCH_TEST}")

add_executable(${STOPWATCH_TEST} stopwatch_tests.cpp)
target_link_libraries(${STOPWATCH_TEST} PRIVATE ${CORVID_UTILS_LIB} ${CORVID_TESTING_LIB})
add_test(NAME StopwatchTest COMMAND ${STOPWATCH_TEST})

message("Stopwatch tests built!")