# Create library using the variable from parent CMakeLists.txt file.
add_library(${CORVID_UTILS_LIB} STATIC)

# Add Boost linkage
target_link_libraries(${CORVID_UTILS_LIB} INTERFACE ${Boost_LIBRARIES})
target_include_directories(${CORVID_UTILS_LIB} SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})

# Add header only files as interface
target_include_directories(${CORVID_UTILS_LIB} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Add compiled files, public headers and private implementations
target_sources(${CORVID_UTILS_LIB} PUBLIC websocket_client.hpp local_log_helper.hpp)
target_sources(${CORVID_UTILS_LIB} PRIVATE websocket_client.cpp local_log_helper.cpp)