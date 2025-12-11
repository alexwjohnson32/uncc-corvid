# 1. Find required packages and libs
find_package(MPI REQUIRED)

find_library(HELICS_LIB NAMES helicscpp HINTS /usr/local/helics/lib64)
set(HELICS_INCLUDE_DIR /usr/local/helics/include)

add_executable(query-federate-exe query-federate.cpp query_federate_input.cpp)

#2. Link libraries
target_link_libraries(query-federate-exe PRIVATE MPI::MPI_CXX ${HELICS_LIB} corvid_helics_lib)
target_include_directories(query-federate-exe PRIVATE ${HELICS_INCLUDE_DIR})

#3. Install
install(TARGETS query-federate-exe DESTINATION ${CMAKE_INSTALL_PREFIX}/corvid_helics_lib/data_federates)
install(FILES query_config.json DESTINATION ${CMAKE_INSTALL_PREFIX}/corvid_helics_lib/data_federates)