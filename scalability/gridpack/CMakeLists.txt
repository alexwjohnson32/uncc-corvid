find_package(MPI REQUIRED)

set(GA_ROOT "/usr/local/ga-5.8")
set(GP_ROOT "/usr/local/GridPACK")
set(HELICS_ROOT "/usr/local/helics")
set(PETSC_LIB_DIR "/usr/local/petsc-3.16.4/lib")

set(GRIDPACK_LIBS
    gridpack_timer gridpack_environment gridpack_powerflow_module
    gridpack_dynamic_simulation_full_y_module gridpack_parallel
    gridpack_partition gridpack_pfmatrix_components gridpack_ymatrix_components
    gridpack_components gridpack_stream gridpack_block_parsers
    gridpack_math gridpack_configuration
)

add_subdirectory(powerflow)
add_subdirectory(IEEE-118)