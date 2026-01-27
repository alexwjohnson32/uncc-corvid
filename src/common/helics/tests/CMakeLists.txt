# Inputs testing
set(INPUT_TEST input_test)

add_executable(${INPUT_TEST} input_tests.cpp)
target_link_libraries(${INPUT_TEST} PRIVATE ${CORVID_HELICS_LIB} ${CORVID_UTILS_LIB} ${CORVID_TESTING_LIB})
add_test(NAME InputTest COMMAND ${INPUT_TEST})