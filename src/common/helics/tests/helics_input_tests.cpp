#include <gtest/gtest.h>
#include <boost/json.hpp>
#include <string>

#include "common/helics/helics_input.hpp"

class HelicsInputTest : public ::testing::Test
{
  protected:
    common::helics::HelicsInput createSampleInput()
    {
        common::helics::HelicsInput input;
        input.federate_name = "SystemFederate";
        input.fed_info_json = "{\"coreType\":\"zmq\"}";
        input.total_time = 86400.0;
        input.local_log_file = "/var/log/helics/test.log";
        return input;
    }

    // Helper to extract JSON from a field that might be a raw string or a nested object
    boost::json::value getNestedJson(const boost::json::value &field)
    {
        if (field.is_string())
        {
            return boost::json::parse(field.as_string());
        }
        return field; // Return as-is if it's already an object/array
    }
};

// Test 1: Verify specific JSON key-value mapping
TEST_F(HelicsInputTest, SerializationCorrectness)
{
    const common::helics::HelicsInput input = createSampleInput();
    const boost::json::value jv = boost::json::value_from(input);

    ASSERT_TRUE(jv.is_object());
    const boost::json::object &obj = jv.as_object();

    // Check federate_name
    ASSERT_TRUE(obj.at("federate_name").is_string());
    EXPECT_EQ(obj.at("federate_name").as_string(), "SystemFederate");

    // Check fed_info_json: It might be a string (serialized) or an object (nested)
    const boost::json::value &fed_info_field = obj.at("fed_info_json");
    boost::json::value parsed_fed_info = getNestedJson(fed_info_field);
    EXPECT_EQ(parsed_fed_info, boost::json::parse("{\"coreType\":\"zmq\"}"));

    // Check total_time
    ASSERT_TRUE(obj.at("total_time").is_number());
    EXPECT_DOUBLE_EQ(obj.at("total_time").as_double(), 86400.0);

    // Check local_log_file
    ASSERT_TRUE(obj.at("local_log_file").is_string());
    EXPECT_EQ(obj.at("local_log_file").as_string(), "/var/log/helics/test.log");
}

// Test 2: Full Round-Trip (Struct -> JSON -> Struct)
TEST_F(HelicsInputTest, RoundTripConsistency)
{
    const common::helics::HelicsInput original = createSampleInput();

    const boost::json::value jv = boost::json::value_from(original);
    const common::helics::HelicsInput decoded = boost::json::value_to<common::helics::HelicsInput>(jv);

    EXPECT_EQ(original.federate_name, decoded.federate_name);

    // Compare semantic JSON content for the nested string/object
    const boost::json::value original_nested = boost::json::parse(original.fed_info_json);
    const boost::json::value decoded_nested = boost::json::parse(decoded.fed_info_json);
    EXPECT_EQ(original_nested, decoded_nested);

    EXPECT_DOUBLE_EQ(original.total_time, decoded.total_time);
    EXPECT_EQ(original.local_log_file, decoded.local_log_file);
}

// Test 3: Verify behavior with default initialized values
TEST_F(HelicsInputTest, DefaultValuesHandling)
{
    common::helics::HelicsInput empty_input;
    empty_input.federate_name = "";
    empty_input.fed_info_json = "{}";
    empty_input.total_time = 0.0;
    empty_input.local_log_file = "";

    const boost::json::value jv = boost::json::value_from(empty_input);
    const common::helics::HelicsInput decoded = boost::json::value_to<common::helics::HelicsInput>(jv);

    EXPECT_EQ(decoded.federate_name, "");
    EXPECT_EQ(boost::json::parse(decoded.fed_info_json), boost::json::parse("{}"));
    EXPECT_DOUBLE_EQ(decoded.total_time, 0.0);
    EXPECT_EQ(decoded.local_log_file, "");
}

// Test 4: Verify manual JSON construction to Struct mapping
TEST_F(HelicsInputTest, ManualJsonMapping)
{
    boost::json::object obj;
    obj["federate_name"] = "ManualFed";
    obj["fed_info_json"] = boost::json::parse("{\"key\":\"value\"}");
    obj["total_time"] = 123.456;
    obj["local_log_file"] = "manual.log";

    const boost::json::value jv(obj);
    const common::helics::HelicsInput decoded = boost::json::value_to<common::helics::HelicsInput>(jv);

    EXPECT_EQ(decoded.federate_name, "ManualFed");

    // Semantic comparison of the resulting string
    EXPECT_EQ(boost::json::parse(decoded.fed_info_json), boost::json::parse("{\"key\":\"value\"}"));

    EXPECT_DOUBLE_EQ(decoded.total_time, 123.456);
    EXPECT_EQ(decoded.local_log_file, "manual.log");
}