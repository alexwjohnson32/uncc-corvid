message(STATUS "Finding Boost 1.78")
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.78 REQUIRED COMPONENTS system filesystem json mpi random)

message(STATUS "Found Boost INCLUDE: ${Boost_INCLUDE_DIRS}")
message(STATUS "Found Boost LIBS: ${Boost_LIBRARIES}")

message(STATUS "Finding MPI")
find_package(MPI REQUIRED)

set(GA_ROOT "/usr/local/ga-5.8")
set(GP_ROOT "/usr/local/GridPACK")
set(GRIDPACK_LIBS
    gridpack_timer gridpack_environment gridpack_powerflow_module
    gridpack_dynamic_simulation_full_y_module gridpack_parallel
    gridpack_partition gridpack_pfmatrix_components gridpack_ymatrix_components
    gridpack_components gridpack_stream gridpack_block_parsers
    gridpack_math gridpack_configuration
)
set(HELICS_ROOT "/usr/local/helics")
set(HELICS_LIB helicscpp)
set(HELICS_INCLUDE_DIR "/usr/local/helics/include")
set(PETSC_LIB_DIR "/usr/local/petsc-3.16.4/lib")
set(GRIDPACK_DEPS_LIBS
    ga++ ga armci parmetis petsc MPI::MPI_CXX
)

message(STATUS "GA_ROOT: ${GA_ROOT}")
message(STATUS "GP_ROOT: ${GP_ROOT}")
message(STATUS "GRIDPACK_LIBS: ${GRIDPACK_LIBS}")
message(STATUS "HELICS_ROOT: ${HELICS_ROOT}")
message(STATUS "HELICS_LIB: ${HELICS_LIB}")
message(STATUS "HELICS_INCLUDE_DIR: ${HELICS_INCLUDE_DIR}")
message(STATUS "PETSC_LIB_DIR: ${PETSC_LIB_DIR}")
message(STATUS "GRIDPACK_DEPS_LIBS: ${GRIDPACK_DEPS_LIBS}")