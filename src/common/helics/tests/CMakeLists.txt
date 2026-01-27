# Inputs testing
set(INPUT_TEST input_test)

add_executable(${INPUT_TEST} input_tests.cpp)
target_link_libraries(${INPUT_TEST} PRIVATE ${CORVID_HELICS_LIB} ${CORVID_UTILS_LIB} ${CORVID_TESTING_LIB})
add_test(NAME InputTest COMMAND ${INPUT_TEST})

# Inputs testing
set(TOOLS_TEST tools_test)

add_executable(${TOOLS_TEST} tools_tests.cpp)
target_link_libraries(${TOOLS_TEST} PRIVATE ${CORVID_HELICS_LIB} ${CORVID_UTILS_LIB} ${CORVID_TESTING_LIB})
add_test(NAME ToolsTest COMMAND ${TOOLS_TEST})