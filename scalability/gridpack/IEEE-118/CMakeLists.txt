set(SOURCES main.cpp ieee_118_app.cpp)

set(PF_NAME powerflow_ex.x)

add_executable(${PF_NAME} ${SOURCES})

target_include_directories(${PF_NAME} PRIVATE ${GA_ROOT}/include ${GP_ROOT}/include ${HELICS_ROOT}/include)
target_link_directories(${PF_NAME} PRIVATE ${PETSC_LIB_DIR} ${GA_ROOT}/lib ${GP_ROOT}/lib ${HELICS_ROOT}/lib64)
target_link_libraries(${PF_NAME} PRIVATE helicscpp ${GRIDPACK_LIBS} ga++ ga armci parmetis petsc MPI::MPI_CXX corvid_helics_lib ${POWERFLOW_LIB_NAME})

install(TARGETS ${PF_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/gridpack/IEEE-118)
install(FILES 118.raw 118.xml helics_setup.json DESTINATION ${CMAKE_INSTALL_PREFIX}/gridpack/IEEE-118)