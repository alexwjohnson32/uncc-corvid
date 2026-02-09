add_library(${CORVID_HELICS_LIB})

target_sources(${CORVID_HELICS_LIB}
               PRIVATE
               tools.cpp helics_input.cpp
               PUBLIC
               FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
               FILES tools.hpp helics_input.hpp)

target_include_directories(${CORVID_HELICS_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${HELICS_ROOT}/include)
target_link_directories(${CORVID_HELICS_LIB} PUBLIC ${HELICS_ROOT}/lib64)
target_link_libraries(${CORVID_HELICS_LIB} PUBLIC helicscpp MPI::MPI_CXX ${CORVID_UTILS_LIB})

if (ENABLE_TESTING)
    # Add tests to compile and link against the libs
    add_subdirectory(tests)
endif()