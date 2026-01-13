add_library(${CORVID_HELICS_LIB} STATIC)

target_sources(${CORVID_HELICS_LIB}
               PRIVATE
               input.cpp tools.cpp
               PUBLIC
               FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
               FILES input.hpp tools.hpp)

target_include_directories(${CORVID_HELICS_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${HELICS_ROOT}/include)
target_link_directories(${CORVID_HELICS_LIB} PUBLIC ${HELICS_ROOT}/lib64)
target_link_libraries(${CORVID_HELICS_LIB} PUBLIC helicscpp MPI::MPI_CXX ${CORVID_UTILS_LIB})