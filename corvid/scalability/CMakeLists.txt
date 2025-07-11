cmake_minimum_required(VERSION 3.26)

project(Scalability)
set(MODULE scalability_exec)

set(CMAKE_CXX_STANDARD 11) # Use this instead of -std=c++11 or -std=c++0x
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Ensure the compiler supports C++11

include(dependencies.cmake)
include(testing.cmake)

add_subdirectory(corvid_helics_lib)
add_subdirectory(gridlabd)
add_subdirectory(gridpack)
add_subdirectory(orchestrator)