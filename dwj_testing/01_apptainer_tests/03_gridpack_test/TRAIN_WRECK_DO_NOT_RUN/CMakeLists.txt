cmake_minimum_required(VERSION 3.10)
project(VerifyGridPACK)

# Point to the config file inside the container
#set(GridPACK_DIR "/usr/local/gridpack/share/gridpack")
#find_package(GridPACK REQUIRED)

add_executable(check_gridpack.x check_gridpack.cpp)

#-------------------------------------------------------------------------
target_include_directories(check_gridpack.x
  PRIVATE
  /usr/local/openmpi-4.1.6/
  /usr/local/openmpi-4.1.6/include/
  /usr/local/parmetis-4.0.3/include
  /usr/local/ga-5.8/include
  /usr/local/gridpack/include
  /usr/local/gridpack/include/gridpack/include/
  /usr/local/boost-1.78.0/include
  )

 
 # List of library names
 set(LIBRARIES_NAMES
   "mpi"
   "petsc"
   "parmetis"
   "boost_random"
   "boost_serialization"
   "boost_mpi"
   "armci"
   "ga"
   "ga++"
   "gridpack_configuration"
   "gridpack_math"
   "gridpack_block_parsers"
   "gridpack_stream"
   "gridpack_components"
   "gridpack_ymatrix_components"
   "gridpack_pfmatrix_components"
   "gridpack_partition"
   "gridpack_parallel"
   "gridpack_dynamic_simulation_full_y_module"
   "gridpack_powerflow_module"
   "gridpack_environment"
   "gridpack_timer"
   )

# Variable to hold the paths of found libraries
set(LIBRARIES_FOUND)

#---SET MPI_CXX_COMPILER
find_package(MPI REQUIRED)
if (MPI_FOUND)
    set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})

#    include_directories(${MPI_INCLUDE_PATH})
#    message("CMAKE_CXX_COMPILED = '${CMAKE_CXX_COMPILER}'")
#    message("MPI_INCLUDE_PATH = '${MPI_INCLUDE_PATH}'")
#else()
#    message("DID NOT WORK")
endif()

#---END TROUBLESHOOTING MPICXX

# Step 1: Loop through the list to find each library
foreach(LIBRARY_NAME IN LISTS LIBRARIES_NAMES)

  message("LIBRARY_NAME = '${LIBRARY_NAME}'")

  find_library(LIBRARY_PATH NAMES ${LIBRARY_NAME}
    PATHS /usr/local/gridpack/lib
    /usr/local/ga-5.8/lib
    /usr/local/petsc-3.16.4/lib
    /usr/local/parmetis-4.0.3/lib
    /usr/local/boost-1.78.0/lib
    /usr/lib64/openmpi/lib
    /usr/local/openmpi-4.1.6/lib
    NO_CACHE
    NO_DEFAULT_PATH
    )

  message("LINBRARY_PATH = '${LIBRARY_PATH}'")

  # Check if the library was found
  if(NOT LIBRARY_PATH)
    message(FATAL_ERROR "Library '${LIBRARY_NAME}' could not be found.")
  endif()

  # Append the found library path to the LIBRARIES_FOUND list
  list(INSERT LIBRARIES_FOUND 0  ${LIBRARY_PATH})
  unset(LIBRARY_PATH)
endforeach()

message(STATUS "All Libraries found: ${LIBRARIES_FOUND}")
#-------------------------------------------------------------------------



#target_link_libraries(check_gridpack.x GridPACK::gridpack)
target_link_libraries(check_gridpack.x PRIVATE ${LIBRARIES_FOUND} )