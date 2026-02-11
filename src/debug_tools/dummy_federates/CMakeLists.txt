set(DUMMY_FEDERATE_NAME dummy_federate)
set(CORVID_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/debug_tools/dummy_federates)

add_executable(${DUMMY_FEDERATE_NAME} main.cpp dummy_federate.cpp)
target_link_libraries(${DUMMY_FEDERATE_NAME} PRIVATE ${CORVID_UTILS_LIB} ${CORVID_HELICS_LIB})

install(TARGETS ${DUMMY_FEDERATE_NAME} DESTINATION ${CORVID_INSTALL_DIR})
install(FILES helics.json README.md DESTINATION ${CORVID_INSTALL_DIR})