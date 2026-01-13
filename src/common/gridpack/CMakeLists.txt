# Create library using the variable from parent CMakeLists.txt file.
add_library(${CORVID_GRIDPACK_LIB} STATIC)

# link against libs as PUBLIC
target_include_directories(${CORVID_GRIDPACK_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${GA_ROOT}/include ${GP_ROOT}/include)
target_link_directories(${CORVID_GRIDPACK_LIB} PUBLIC ${PETSC_LIB_DIR} ${GA_ROOT}/lib ${GP_ROOT}/lib)
target_link_libraries(${CORVID_GRIDPACK_LIB} PUBLIC ${GRIDPACK_LIBS} ${GRIDPACK_DEPS_LIBS} MPI::MPI_CXX ${CORVID_UTILS_LIB})