cmake_minimum_required(VERSION 3.10)
project(BasicGpkCosim CXX)

# Force C++17 as required by HELICS 3
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find MPI (Standard for GridPACK/GA)
find_package(MPI REQUIRED)

# 2. Find HELICS
# We look for HELICS 3 specifically.
find_package(HELICS 3 REQUIRED)

# 3. Create the Executable
add_executable(gpk_fed gpk_fed.cpp)

# 4. Include Directories
target_include_directories(gpk_fed PRIVATE 
    ${MPI_CXX_INCLUDE_DIRS}
    ${HELICS_INCLUDE_DIRS}
)

# 5. Link Libraries
# We link the C++ wrapper (helicscpp) and the core library.
# We also link MPI_CXX to ensure GridPACK-style rank handling.
target_link_libraries(gpk_fed PRIVATE 
    HELICS::helicscpp
    HELICS::helics
    MPI::MPI_CXX
)