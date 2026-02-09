# Inputs testing
set(HELICS_INPUT_TEST helics_input_test)

add_executable(${HELICS_INPUT_TEST} helics_input_tests.cpp)
target_link_libraries(${HELICS_INPUT_TEST} PRIVATE ${CORVID_HELICS_LIB} ${CORVID_UTILS_LIB} ${CORVID_TESTING_LIB})
add_test(NAME HelicsInputTest COMMAND ${HELICS_INPUT_TEST})

# Tools testing
set(TOOLS_TEST tools_test)

add_executable(${TOOLS_TEST} tools_tests.cpp)
target_link_libraries(${TOOLS_TEST} PRIVATE ${CORVID_HELICS_LIB} ${CORVID_UTILS_LIB} ${CORVID_TESTING_LIB})
add_test(NAME ToolsTest COMMAND ${TOOLS_TEST})