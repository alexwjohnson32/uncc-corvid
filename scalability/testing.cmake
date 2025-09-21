find_package(GTest REQUIRED)

message(STATUS "Found GTest INCLUDE: ${GTEST_INCLUDE_DIRS}")
message(STATUS "Found GTest LIBS: ${GTEST_BOTH_LIBRARIES}")

enable_testing()