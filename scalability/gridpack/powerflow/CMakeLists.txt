set(POWERFLOW_LIB_NAME powerflow_lib)

add_library(${POWERFLOW_LIB_NAME})

target_include_directories(${POWERFLOW_LIB_NAME} PRIVATE ${GA_ROOT}/include ${GP_ROOT}/include ${HELICS_ROOT}/include)
target_link_directories(${POWERFLOW_LIB_NAME} PRIVATE ${PETSC_LIB_DIR} ${GA_ROOT}/lib ${GP_ROOT}/lib ${HELICS_ROOT}/lib64)
target_link_libraries(${POWERFLOW_LIB_NAME} PRIVATE helicscpp ${GRIDPACK_LIBS} ga++ ga armci parmetis petsc MPI::MPI_CXX corvid_helics_lib)

target_sources(${POWERFLOW_LIB_NAME} PUBLIC factory.hpp input.hpp state.hpp tools.hpp)
target_sources(${POWERFLOW_LIB_NAME} PRIVATE factory.cpp input.cpp state.cpp tools.cpp)