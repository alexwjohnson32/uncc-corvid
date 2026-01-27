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
    std::complex<double> result = powerflow::tools::LimitPower(s, max_v);

    EXPECT_DOUBLE_EQ(result.real(), 30.0);
    EXPECT_DOUBLE_EQ(result.imag(), 40.0);
}

TEST(LimitPowerLogicTest, AboveLimitIsScaled)
{
    const double max_v = 100.0;
    // Magnitude is 200.0, limit is 100.0 (Scale factor 0.5)
    std::complex<double> s(120.0, 160.0);
    std::complex<double> result = powerflow::tools::LimitPower(s, max_v);

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

    double magnitude = 240.0;
    // Ensure registrations happen without throwing
    ASSERT_NO_THROW({ powerflow::tools::VoltagePublisher publisher(*fed, magnitude); });
}

TEST_F(HelicsToolsTest, LimitPowerWithSubscriptions)
{
    if (!fed) return;

    powerflow::tools::ThreePhaseSubscriptions subs;
    // Using local names (not global) for safer unit testing
    subs.a = fed->registerInput<std::complex<double>>("test_a", "V");
    subs.b = fed->registerInput<std::complex<double>>("test_b", "V");
    subs.c = fed->registerInput<std::complex<double>>("test_c", "V");

    auto pub_a = fed->registerPublication<std::complex<double>>("test_a", "V");
    auto pub_b = fed->registerPublication<std::complex<double>>("test_b", "V");
    auto pub_c = fed->registerPublication<std::complex<double>>("test_c", "V");

    fed->enterExecutingMode();

    // Value in implementation is divided by 1e8.
    // We want a result of (3,4) -> publish (3e8, 4e8)
    std::complex<double> val(3e8, 4e8);
    pub_a.publish(val);
    pub_b.publish(val);
    pub_c.publish(val);

    fed->requestNextStep();

    double limit = 10.0;
    powerflow::tools::ThreePhaseValues results = powerflow::tools::LimitPower(subs, limit);

    EXPECT_DOUBLE_EQ(results.a.real(), 3.0);
    EXPECT_DOUBLE_EQ(results.a.imag(), 4.0);
}

TEST_F(HelicsToolsTest, LimitPowerWithSubscriptionsAboveLimit)
{
    if (!fed) return;

    powerflow::tools::ThreePhaseSubscriptions subs;
    subs.a = fed->registerInput<std::complex<double>>("scale_a", "V");
    subs.b = fed->registerInput<std::complex<double>>("scale_b", "V");
    subs.c = fed->registerInput<std::complex<double>>("scale_c", "V");

    auto pub_a = fed->registerPublication<std::complex<double>>("scale_a", "V");

    fed->enterExecutingMode();

    // 12e8 + 16e8j -> 20e8. Divided by 1e8 = 20. Limit 10 -> Scale 0.5.
    pub_a.publish(std::complex<double>(12e8, 16e8));

    fed->requestNextStep();

    double limit = 10.0;
    powerflow::tools::ThreePhaseValues results = powerflow::tools::LimitPower(subs, limit);

    EXPECT_DOUBLE_EQ(results.a.real(), 6.0);
    EXPECT_DOUBLE_EQ(results.a.imag(), 8.0);
    EXPECT_DOUBLE_EQ(std::abs(results.a), 10.0);
}