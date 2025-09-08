cmake_minimum_required(VERSION 3.21)
project(PowerFlow)

enable_language(CXX)

add_executable(gpk-left-fed.x
  gpk-left-fed.cpp
  pf_app.cpp
  pf_factory.cpp
  )

target_include_directories(gpk-left-fed.x
  PRIVATE
  /usr/include/openmpi-x86_64
  /usr/lib/x86_64-linux-gnu/openmpi/include
  /usr/local/ga-5.8/include
  /usr/local/GridPACK/include
  /usr/local/helics/include
  )

 
 # List of library names
 set(LIBRARIES_NAMES
   "mpi"
   "mpi_cxx"
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
   "helicscpp"
   )

# Variable to hold the paths of found libraries
set(LIBRARIES_FOUND)

# Step 1: Loop through the list to find each library
foreach(LIBRARY_NAME IN LISTS LIBRARIES_NAMES)

  find_library(LIBRARY_PATH NAMES ${LIBRARY_NAME}
    PATHS /usr/local/GridPACK/lib
    /usr/local/ga-5.8/lib
    /usr/local/petsc-3.16.4/lib
    /usr/local/boost-1.78.0/lib
    /usr/lib64/openmpi/lib
    /usr/lib/x86_64-linux-gnu/openmpi/lib
    /usr/local/helics/lib
    NO_CACHE
    NO_DEFAULT_PATH
    )

  # Check if the library was found
  if(NOT LIBRARY_PATH)
    message(FATAL_ERROR "Library '${LIBRARY_NAME}' could not be found.")
  endif()

  # Append the found library path to the LIBRARIES_FOUND list
  list(INSERT LIBRARIES_FOUND 0  ${LIBRARY_PATH})
  unset(LIBRARY_PATH)
endforeach()

message(STATUS "All Libraries found: ${LIBRARIES_FOUND}")


# Step 2: Link all found libraries to the executable
target_link_libraries(gpk-left-fed.x PRIVATE ${LIBRARIES_FOUND} )

# Copy example files
add_custom_command(
    TARGET gpk-left-fed.x POST_BUILD  # Run after the executable is built
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/input.xml" "${CMAKE_BINARY_DIR}"
    COMMENT "Copying xml configuration to the build directory"
    )

add_custom_command(
    TARGET gpk-left-fed.x POST_BUILD  # Run after the executable is built
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/IEEE14.raw" "${CMAKE_BINARY_DIR}"
    COMMENT "Copying network file to the build directory"
    )

add_custom_command(
    TARGET gpk-left-fed.x POST_BUILD  # Run after the executable is built
    COMMAND ${CMAKE_COMMAND} -E copy
         "${CMAKE_SOURCE_DIR}/118.xml" "${CMAKE_BINARY_DIR}"
    COMMENT "118 xml file"
    )

add_custom_command(
    TARGET gpk-left-fed.x POST_BUILD  # Run after the executable is built
    COMMAND ${CMAKE_COMMAND} -E copy
         "${CMAKE_SOURCE_DIR}/118.raw" "${CMAKE_BINARY_DIR}"
    COMMENT "118 xml file"
    )

add_custom_command(
    TARGET gpk-left-fed.x POST_BUILD  # Run after the executable is built
    COMMAND ${CMAKE_COMMAND} -E copy
         "${CMAKE_SOURCE_DIR}/oneline.raw" "${CMAKE_BINARY_DIR}"
    COMMENT "oneline.raw"
    )


