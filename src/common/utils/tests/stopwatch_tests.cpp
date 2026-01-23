#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "stopwatch.hpp"

class StopwatchTest : public ::testing::Test
{
};

/**
 * @brief Verifies that the constructor automatically calls Start().
 * * The elapsed time immediately after construction should be nearly zero.
 */
TEST_F(StopwatchTest, InitialStateIsStarted)
{
    utils::Stopwatch sw;
    double elapsed = sw.ElapsedMilliseconds();

    // The elapsed time should be non-negative and very close to zero
    // (typically < 1ms unless the system is under extreme load).
    EXPECT_GE(elapsed, 0.0);
    EXPECT_LT(elapsed, 5.0);
}

/**
 * @brief Verifies that ElapsedMilliseconds tracks time roughly correctly.
 * * We use a sleep period and then check if the elapsed time is within a
 * reasonable margin of error (accounting for OS scheduling/jitter).
 */
TEST_F(StopwatchTest, MeasuresElapsedSleepTime)
{
    utils::Stopwatch sw;
    const int sleep_ms = 100;

    // We don't strictly need to call sw.Start() here because of the constructor,
    // but doing so ensures we are testing the measurement from this exact point.
    sw.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    double elapsed = sw.ElapsedMilliseconds();

    EXPECT_GE(elapsed, static_cast<double>(sleep_ms));
    EXPECT_NEAR(elapsed, sleep_ms, 25.0);
}

/**
 * @brief Verifies that calling Start() multiple times resets the reference point.
 */
TEST_F(StopwatchTest, ResetFunctionality)
{
    utils::Stopwatch sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Restarting the stopwatch
    sw.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    double elapsed = sw.ElapsedMilliseconds();

    // It should reflect the time since the manual Start() call, not the constructor.
    EXPECT_NEAR(elapsed, 30.0, 15.0);
    EXPECT_LT(elapsed, 50.0);
}

/**
 * @brief Ensures that multiple calls to ElapsedMilliseconds show increasing time.
 */
TEST_F(StopwatchTest, MonotonicIncrease)
{
    utils::Stopwatch sw;

    double t1 = sw.ElapsedMilliseconds();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    double t2 = sw.ElapsedMilliseconds();

    EXPECT_LE(t1, t2);
}

// Standard GTest main entry point
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}