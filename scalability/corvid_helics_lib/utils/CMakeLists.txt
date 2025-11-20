target_include_directories(corvid_helics_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(corvid_helics_lib PUBLIC persistent-websocket-client.hpp)
target_sources(corvid_helics_lib PRIVATE persistent-websocket-client.cpp)