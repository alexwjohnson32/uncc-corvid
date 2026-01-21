set(EXEC_NAME ieee-118-gridpack-federate)
set(CORVID_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/federate/gridpack/IEEE-118)

add_executable(${EXEC_NAME} main.cpp ieee_118_app.cpp)
target_link_libraries(${EXEC_NAME} PRIVATE ${CORVID_UTILS_LIB} ${CORVID_GRIDPACK_LIB} ${CORVID_HELICS_LIB})

install(TARGETS ${EXEC_NAME} DESTINATION ${CORVID_INSTALL_DIR})
install(FILES helics_setup.json 118.raw 118.xml DESTINATION ${CORVID_INSTALL_DIR})