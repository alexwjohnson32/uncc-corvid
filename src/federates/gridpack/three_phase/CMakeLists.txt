set(EXEC_NAME three-phase-gridpack-federate)
set(CORVID_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/federate/gridpack/three_phase)

add_executable(${EXEC_NAME} main.cpp federate.cpp app.cpp inputs.cpp federate_state.cpp)
target_link_libraries(${EXEC_NAME} PRIVATE ${CORVID_UTILS_LIB} ${CORVID_GRIDPACK_LIB} ${CORVID_HELICS_LIB})

install(TARGETS ${EXEC_NAME} DESTINATION ${CORVID_INSTALL_DIR})
install(FILES helics_setup.json README.md DESTINATION ${CORVID_INSTALL_DIR})

add_subdirectory(raw_files)
add_subdirectory(xml_files)