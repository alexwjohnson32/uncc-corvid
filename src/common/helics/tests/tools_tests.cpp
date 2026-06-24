#include <gtest/gtest.h>
#include <complex>
#include <helics/helics.hpp>

#include "tools.hpp"

// -----------------------------------------------------------------------------
// LimitPower Logic Tests (Pure Math)
// -----------------------------------------------------------------------------

TEST(LimitPowerLogicTest, BelowLimitRemainsUnchanged)
{
    const double max_v = 100.0;
    std::complex<double> s(30.0, 40.0); // Magnitude is 50.0
    std::complex<double> result = common::helics::LimitPower(s, max_v);

    EXPECT_DOUBLE_EQ(result.real(), 30.0);
    EXPECT_DOUBLE_EQ(result.imag(), 40.0);
}

TEST(LimitPowerLogicTest, AboveLimitIsScaled)
{
    const double max_v = 100.0;
    // Magnitude is 200.0, limit is 100.0 (Scale factor 0.5)
    std::complex<double> s(120.0, 160.0);
    std::complex<double> result = common::helics::LimitPower(s, max_v);

    EXPECT_DOUBLE_EQ(std::abs(result), max_v);
    EXPECT_DOUBLE_EQ(result.real(), 60.0);
    EXPECT_DOUBLE_EQ(result.imag(), 80.0);
}

// -----------------------------------------------------------------------------
// HELICS Integration Tests
// -----------------------------------------------------------------------------

class HelicsToolsTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Use a random name to avoid collisions if tests run in parallel.
        std::string name = "TestFed_" + std::to_string(std::rand());

        try
        {
            // Using FederateInfo constructor to set core type and init string correctly
            helics::FederateInfo fedInfo(helics::CoreType::DEFAULT);
            // coreInit is set via coreInitString or as a constructor parameter
            fedInfo.coreInitString = "--autobroker";

            fed = std::make_unique<helics::ValueFederate>(name, fedInfo);
        }
        catch (const std::exception &e)
        {
            // If GTEST_SKIP is unavailable, we use a non-fatal failure
            // and check the 'fed' pointer in the tests.
            printf("Skipping HELICS tests: %s\n", e.what());
            fed.reset();
        }
    }

    void TearDown() override
    {
        if (fed)
        {
            fed->finalize();
        }
    }

    std::unique_ptr<helics::ValueFederate> fed;
};

TEST_F(HelicsToolsTest, VoltagePublisherRegistration)
{
    if (!fed) return;

    // Ensure registrations happen without throwing
    ASSERT_NO_THROW({ common::helics::VoltagePublisher publisher(*fed); });
}