cmake_minimum_required(VERSION 3.21)
project(PowerFlow LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MPI REQUIRED)  # <--- add this

add_executable(powerflow_ex.x
  powerflow_ex.cpp
  pf_app.cpp
  pf_factory.cpp
)

# keep your other include dirs if you need them
target_include_directories(powerflow_ex.x PRIVATE
  /usr/local/ga-5.8/include
  /usr/local/GridPACK/include
  /usr/local/helics/include
)

# Remove mpi/mpi_cxx from this list:
set(LIBRARIES_NAMES
  "petsc" "parmetis"
  "boost_random" "boost_serialization" "boost_mpi"
  "armci" "ga" "ga++"
  "gridpack_configuration" "gridpack_math" "gridpack_block_parsers"
  "gridpack_stream" "gridpack_components" "gridpack_ymatrix_components"
  "gridpack_pfmatrix_components" "gridpack_partition" "gridpack_parallel"
  "gridpack_dynamic_simulation_full_y_module" "gridpack_powerflow_module"
  "gridpack_environment" "gridpack_timer"
  "helicscpp"
)

# ... keep your find_library loop for the non-MPI libs ...

# Link MPI via the imported target:
target_link_libraries(powerflow_ex.x PRIVATE MPI::MPI_CXX ${LIBRARIES_FOUND})
