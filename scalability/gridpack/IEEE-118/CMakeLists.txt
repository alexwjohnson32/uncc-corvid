add_executable(powerflow_ex.x
  powerflow_ex.cpp
  pf_app.cpp
  pf_factory.cpp
  pf_input.cpp
  pf_state.cpp
  three_phase_pf_app.cpp
  )

target_include_directories(powerflow_ex.x
  PRIVATE
  /usr/include/openmpi-x86_64
  /usr/local/ga-5.8/include
  /usr/local/GridPACK/include
  /usr/local/helics/include
  )

set(LIBRARIES_NAMES
   "mpi"
   "mpi_cxx"
   "petsc"
   "parmetis"
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
   "helicscpp"
   )

# Variable to hold the paths of found libraries
set(LIBRARIES_FOUND)

# Step 1: Loop through the list to find each library
foreach(LIBRARY_NAME IN LISTS LIBRARIES_NAMES)

  find_library(LIBRARY_PATH NAMES ${LIBRARY_NAME}
    PATHS
    /usr/lib64/openmpi/lib
    /usr/local/petsc-3.16.4/lib
    /usr/local/ga-5.8/lib
    /usr/local/GridPACK/lib
    /usr/local/helics/lib64
    NO_CACHE
    NO_DEFAULT_PATH
    )

  # Check if the library was found
  if(NOT LIBRARY_PATH)
    message(FATAL_ERROR "'${LIBRARY_NAME}' could not be found.")
  else()
    # message(STATUS "'${LIBRARY_NAME}' found at '${LIBRARY_PATH}`.")
  endif()

  # Append the found library path to the LIBRARIES_FOUND list
  list(INSERT LIBRARIES_FOUND 0  ${LIBRARY_PATH})

  unset(LIBRARY_PATH)
endforeach()

# Step 2: Link all found libraries to the executable
target_link_libraries(powerflow_ex.x PRIVATE ${LIBRARIES_FOUND} corvid_helics_lib)

install(TARGETS powerflow_ex.x DESTINATION ${CMAKE_INSTALL_PREFIX}/gridpack/IEEE-118)
install(FILES 118.raw 118.xml helics_fed_info_config.json DESTINATION ${CMAKE_INSTALL_PREFIX}/gridpack/IEEE-118)