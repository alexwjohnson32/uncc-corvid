target_include_directories(corvid_helics_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(corvid_helics_lib PUBLIC persistent-websocket-client.hpp)
target_sources(corvid_helics_lib PRIVATE persistent-websocket-client.cpp)

add_executable(testing_client dummy_client.cpp)
target_link_libraries(testing_client corvid_helics_lib)

add_executable(testing-server testing-websocket-server.cpp)
target_link_libraries(testing-server corvid_helics_lib)