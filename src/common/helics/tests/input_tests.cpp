#include <gtest/gtest.h>
#include <gtest/gtest-spi.h> // Required for EXPECT_NONFATAL_FAILURE
#include <boost/json.hpp>
#include <vector>
#include <string>

#include "input.hpp"

/**
 * @brief Helper to compare two JSON strings semantically.
 * * This avoids failures caused by whitespace differences by parsing both
 * strings into boost::json::value objects before comparison.
 */
void ExpectJsonStringsSemanticEqual(const std::string &actual, const std::string &expected)
{
    try
    {
        boost::json::value v_actual = boost::json::parse(actual);
        boost::json::value v_expected = boost::json::parse(expected);
        EXPECT_EQ(v_actual, v_expected);
    }
    catch (const std::exception &e)
    {
        ADD_FAILURE() << "JSON Parsing failed during comparison: " << e.what() << "\n  Actual string: " << actual
                      << "\n  Expected string: " << expected;
    }
}

class PowerflowInputTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        gl_input.bus_id = 101;
        gl_input.names = { "Node_A", "Node_B" };

        sample_input.gridpack_name = "System_Alpha";
        // Whitespace here shouldn't break our semantic comparison
        sample_input.fed_info_json = "{\"key\": \"value\"}";
        sample_input.total_time = 3600.0;
        sample_input.ln_magnitude = 1.05;
        sample_input.gridlabd_infos = { gl_input };
    }

    powerflow::input::GridlabDInputs gl_input;
    powerflow::input::PowerflowInput sample_input;
};

// -----------------------------------------------------------------------------
// Core Functionality Tests
// -----------------------------------------------------------------------------

TEST_F(PowerflowInputTest, GetGridlabDNamesAggregatesCorrectly)
{
    std::vector<std::string> expected = { "Node_A", "Node_B" };
    EXPECT_EQ(sample_input.GetGridalabDNames(), expected);
}

TEST_F(PowerflowInputTest, PowerflowInputRoundTrip)
{
    // Round trip: Struct -> JSON -> Struct
    boost::json::value jv = boost::json::value_from(sample_input);
    auto decoded = boost::json::value_to<powerflow::input::PowerflowInput>(jv);

    EXPECT_EQ(decoded.gridpack_name, sample_input.gridpack_name);
    EXPECT_DOUBLE_EQ(decoded.total_time, sample_input.total_time);

    // Use the semantic helper for the nested JSON string
    ExpectJsonStringsSemanticEqual(decoded.fed_info_json, sample_input.fed_info_json);

    ASSERT_EQ(decoded.gridlabd_infos.size(), 1);
    EXPECT_EQ(decoded.gridlabd_infos[0].bus_id, gl_input.bus_id);
    EXPECT_EQ(decoded.gridlabd_infos[0].names, gl_input.names);
}

// -----------------------------------------------------------------------------
// Error Handling & Edge Cases
// -----------------------------------------------------------------------------

TEST_F(PowerflowInputTest, ThrowsOnIncompatibleJsonType)
{
    // Providing an array [1, 2] instead of an Object { ... }
    boost::json::value invalid_type = { 1, 2 };

    EXPECT_THROW({ boost::json::value_to<powerflow::input::PowerflowInput>(invalid_type); }, std::exception);
}

/**
 * @brief This test ensures that our helper actually reports an error
 * when the JSON is invalid.
 */
TEST_F(PowerflowInputTest, HelperReportsErrorOnInvalidJson)
{
    // EXPECT_NONFATAL_FAILURE is used to test that a Google Test failure
    // was emitted by the code inside the block.
    EXPECT_NONFATAL_FAILURE({ ExpectJsonStringsSemanticEqual("not json", "{}"); }, "JSON Parsing failed");
}

/**
 * @brief Ensures that valid but differently formatted JSON strings
 * do NOT cause a failure.
 */
TEST_F(PowerflowInputTest, HelperHandlesWhitespaceVariations)
{
    std::string tight = "{\"a\":1}";
    std::string loose = "{  \"a\"  :  1  }";

    // This macro takes only one argument: the statement to execute.
    EXPECT_NO_FATAL_FAILURE({ ExpectJsonStringsSemanticEqual(tight, loose); });
}