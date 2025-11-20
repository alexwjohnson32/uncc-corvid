# Step 1: Loop through the list to find each library
set(LIBRARIES_NAMES
   "mpi"
   "mpi_cxx"
   "helicscpp"
   )

# Variable to hold the paths of found libraries
set(LIBRARIES_FOUND)

foreach(LIBRARY_NAME IN LISTS LIBRARIES_NAMES)

  find_library(LIBRARY_PATH NAMES ${LIBRARY_NAME}
    PATHS
    /usr/lib64/openmpi/lib
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

# Step 2: Add executables
add_executable(query-federate-exe query-federate.cpp)
target_include_directories(query-federate-exe
  PRIVATE
  /usr/include/openmpi-x86_64
  /usr/local/helics/include
  )

add_executable(websocket-server-exe websocket-server.cpp)

# Step 3: Link all found libraries to the executables
target_link_libraries(query-federate-exe ${LIBRARIES_FOUND} corvid_helics_lib)
target_link_libraries(websocket-server-exe corvid_helics_lib)

# Step 4: Install built objects
install(TARGETS query-federate-exe DESTINATION ${CMAKE_INSTALL_PREFIX}/corvid_helics_lib/data_federates)
install(FILES query_config.json DESTINATION ${CMAKE_INSTALL_PREFIX}/corvid_helics_lib/data_federates)
install(TARGETS websocket-server-exe DESTINATION ${CMAKE_INSTALL_PREFIX}/corvid_helics_lib/data_federates)