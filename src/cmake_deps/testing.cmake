# We assume this only gets called if it has already been determined to build with it.
# Meaning, don't do any kind of build checks for testing within here.
find_package(GTest REQUIRED)

message(STATUS "Found GTest INCLUDE: ${GTEST_INCLUDE_DIRS}")
message(STATUS "Found GTest LIBS: ${GTEST_BOTH_LIBRARIES}")

find_package(Threads REQUIRED) # This is used by Gtest for threading

enable_testing()

set(CORVID_TESTING_LIB corvid_testing_lib)
add_library(${CORVID_TESTING_LIB} INTERFACE)
target_link_libraries(${CORVID_TESTING_LIB} INTERFACE GTest::gtest GTest::gtest_main Threads::Threads)