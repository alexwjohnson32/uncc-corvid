find_package(MPI REQUIRED)

set(GA_ROOT "/usr/local/ga-5.8")
set(GP_ROOT "/usr/local/GridPACK")
set(HELICS_ROOT "/usr/local/helics")
set(PETSC_LIB_DIR "/usr/local/petsc-3.16.4/lib")
set(SOURCES powerflow_ex.cpp pf_app.cpp pf_factory.cpp pf_input.cpp pf_state.cpp three_phase_pf_app.cpp)

set(GRIDPACK_LIBS
    gridpack_timer gridpack_environment gridpack_powerflow_module
    gridpack_dynamic_simulation_full_y_module gridpack_parallel
    gridpack_partition gridpack_pfmatrix_components gridpack_ymatrix_components
    gridpack_components gridpack_stream gridpack_block_parsers
    gridpack_math gridpack_configuration
)

set(PF_NAME powerflow_ex.x)

add_executable(${PF_NAME} ${SOURCES})

target_include_directories(${PF_NAME} PRIVATE ${GA_ROOT}/include ${GP_ROOT}/include ${HELICS_ROOT}/include)
target_link_directories(${PF_NAME} PRIVATE ${PETSC_LIB_DIR} ${GA_ROOT}/lib ${GP_ROOT}/lib ${HELICS_ROOT}/lib64)
target_link_libraries(${PF_NAME} PRIVATE helicscpp ${GRIDPACK_LIBS} ga++ ga armci parmetis petsc MPI::MPI_CXX corvid_helics_lib)

install(TARGETS ${PF_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/gridpack/IEEE-118)
install(FILES 118.raw 118.xml helics_setup.json DESTINATION ${CMAKE_INSTALL_PREFIX}/gridpack/IEEE-118)