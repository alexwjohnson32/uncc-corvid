add_library(${POWERFLOW_LIB_NAME} STATIC)

target_sources(${POWERFLOW_LIB_NAME} PRIVATE input.cpp state.cpp tools.cpp
                                     PUBLIC FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FILES
                                     input.hpp state.hpp tools.hpp)

target_include_directories(${POWERFLOW_LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${GA_ROOT}/include ${GP_ROOT}/include ${HELICS_ROOT}/include)
target_link_directories(${POWERFLOW_LIB_NAME} PUBLIC ${PETSC_LIB_DIR} ${GA_ROOT}/lib ${GP_ROOT}/lib ${HELICS_ROOT}/lib64)
target_link_libraries(${POWERFLOW_LIB_NAME} PUBLIC helicscpp ${GRIDPACK_LIBS} ga++ ga armci parmetis petsc MPI::MPI_CXX corvid_helics_lib)