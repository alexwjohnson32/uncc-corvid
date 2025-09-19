cmake_minimum_required(VERSION 3.26)

project(Scalability)

enable_language(CXX)

set(MODULE scalability_exec)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(dependencies.cmake)
include(testing.cmake)

add_subdirectory(corvid_helics_lib)
add_subdirectory(gridlabd)
add_subdirectory(gridpack)