set(CORVID_HELICS_LIB corvid_helics_lib)

add_library(${CORVID_HELICS_LIB} INTERFACE)

# link boost this way to silence warnings
target_link_libraries(${CORVID_HELICS_LIB} INTERFACE ${Boost_LIBRARIES})
target_include_directories(${CORVID_HELICS_LIB} SYSTEM INTERFACE ${Boost_INCLUDE_DIRS})

add_subdirectory(utils)