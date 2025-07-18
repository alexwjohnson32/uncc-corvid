add_library(corvid_helics_lib INTERFACE)

# link boost this way to silence warnings
target_link_libraries(corvid_helics_lib INTERFACE ${Boost_LIBRARIES})
target_include_directories(corvid_helics_lib SYSTEM INTERFACE ${Boost_INCLUDE_DIRS})

add_subdirectory(utils)