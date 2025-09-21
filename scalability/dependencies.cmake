set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.78 REQUIRED COMPONENTS system filesystem json mpi random)

message(STATUS "Found Boost INCLUDE: ${Boost_INCLUDE_DIRS}")
message(STATUS "Found Boost LIBS: ${Boost_LIBRARIES}")