set(EXEC_NAME queryable-federate)
set(CORVID_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/federate/helics/queryable)

add_executable(${EXEC_NAME} main.cpp query_federate.cpp query_federate_input.cpp)
target_link_libraries(${EXEC_NAME} PRIVATE ${CORVID_HELICS_LIB} ${CORVID_UTILS_LIB})

install(TARGETS ${EXEC_NAME} DESTINATION ${CORVID_INSTALL_DIR})
install(FILES query_config.json DESTINATION ${CORVID_INSTALL_DIR})