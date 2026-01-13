set(PF_NAME powerflow_ex.x)
set(SOURCES main.cpp ieee_118_app.cpp)

add_executable(${PF_NAME} ${SOURCES})

target_link_libraries(${PF_NAME} PRIVATE ${POWERFLOW_LIB_NAME})
target_include_directories(${PF_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS ${PF_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/gridpack/IEEE-118)
install(FILES 118.raw 118.xml helics_setup.json DESTINATION ${CMAKE_INSTALL_PREFIX}/gridpack/IEEE-118)