cmake_minimum_required(VERSION 3.26)

project(Corvid_Cosim)

enable_language(CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake_deps/dependencies.cmake)
include(cmake_deps/testing.cmake)
include(cmake_deps/lib_names.cmake)

add_subdirectory(common)
add_subdirectory(federates)
add_subdirectory(debug_tools)