#include <gtest/gtest.h>
#include "json_templates.hpp" // Replace with the actual name of your header file
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

// --- Test Support Types ---

enum class ScopedEnum : uint16_t
{
    ValA = 10,
    ValB = 20
};

enum UnscopedEnum
{
    UValX = 1,
    UValY = 2
};

// --- Helper Function Tests ---

TEST(UtilsTest, GetUnderlyingType)
{
    EXPECT_EQ(utils::get_underlying_type(ScopedEnum::ValA), 10);
    EXPECT_EQ(utils::get_underlying_type(UValX), 1);
}

TEST(UtilsTest, ToVector)
{
    // C-style array
    int c_arr[] = { 1, 2, 3 };
    auto v1 = utils::to_vector(c_arr);
    EXPECT_EQ(v1.size(), 3);
    EXPECT_EQ(v1[0], 1);

    // std::array
    std::array<std::string, 2> s_arr = { "hello", "world" };
    auto v2 = utils::to_vector(s_arr);
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2[1], "world");
}

// --- Extraction Tests ---

TEST(UtilsExtractTest, ExtractRawJson)
{
    boost::json::object obj = { { "key", { { "inner", 1 } } } };
    std::string result;
    utils::extract(obj, "key", utils::raw_json(result));

    // Result should be serialized JSON string
    EXPECT_NE(result.find("\"inner\":1"), std::string::npos);
}

TEST(UtilsExtractTest, ExtractCArray)
{
    boost::json::object obj = { { "scores", { 10, 20, 30 } } };
    int scores[3] = { 0, 0, 0 };

    utils::extract(obj, "scores", scores);
    EXPECT_EQ(scores[0], 10);
    EXPECT_EQ(scores[1], 20);
    EXPECT_EQ(scores[2], 30);

    // Test missing key (should zero-initialize)
    utils::extract(obj, "missing", scores);
    EXPECT_EQ(scores[0], 0);
}

TEST(UtilsExtractTest, ExtractEnum)
{
    boost::json::object obj = { { "val", 20 }, { "invalid", "not_an_int" } };
    ScopedEnum e = ScopedEnum::ValA;

    // Successful conversion
    utils::extract(obj, "val", e);
    EXPECT_EQ(e, ScopedEnum::ValB);

    // Invalid conversion (should fallback to 0)
    utils::extract(obj, "invalid", e);
    EXPECT_EQ(static_cast<int>(e), 0);

    // Missing key
    utils::extract(obj, "missing", e);
    EXPECT_EQ(static_cast<int>(e), 0);
}

TEST(UtilsExtractTest, ExtractStandardTypes)
{
    boost::json::object obj = { { "name", "test" }, { "ids", { 1, 2, 3 } }, { "active", true } };

    std::string name;
    std::vector<int> ids;
    bool active = false;

    utils::extract(obj, "name", name);
    utils::extract(obj, "ids", ids);
    utils::extract(obj, "active", active);

    EXPECT_EQ(name, "test");
    ASSERT_EQ(ids.size(), 3);
    EXPECT_EQ(ids[2], 3);
    EXPECT_TRUE(active);
}

// --- String and File I/O Tests ---

TEST(UtilsIOTest, StringConversion)
{
    std::vector<int> data = { 1, 2, 3 };

    std::string json = utils::ToJsonString(data);
    EXPECT_EQ(json, "[1,2,3]");

    auto parsed = utils::FromJsonString<std::vector<int>>(json);
    EXPECT_EQ(parsed, data);
}

TEST(UtilsIOTest, FormatPretty)
{
    std::string raw = "{\"a\":1,\"b\":[1,2]}";
    std::string pretty = utils::FormatPretty(raw);

    // Property tree adds spaces and newlines
    EXPECT_NE(pretty.find("\n"), std::string::npos);
    EXPECT_NE(pretty.find("    "), std::string::npos);
}

TEST(UtilsIOTest, FileOperations)
{
    std::string filename = "test_output.json";
    boost::json::object data = { { "id", 101 }, { "status", "ok" } };

    // Test Writing
    ASSERT_TRUE(utils::ToJsonFile(data, filename, true));
    EXPECT_TRUE(fs::exists(filename));

    // Test Read as String (Pretty)
    std::string content = utils::ReadJsonFileAsString(filename, true);
    EXPECT_NE(content.find("\"id\": \"101\""), std::string::npos); // ptree often quotes numbers

    // Test Reading into Object
    try
    {
        auto read_data = utils::FromJsonFile<boost::json::object>(filename);
        EXPECT_EQ(std::stoi(static_cast<std::string>(read_data["id"].as_string())), 101);
    }
    catch (const std::exception &e)
    {
        FAIL() << "FromJsonFile threw an exception: " << e.what();
    }

    // Cleanup
    fs::remove(filename);
}

TEST(UtilsIOTest, FileErrorHandling)
{
    // Non-existent directory
    EXPECT_FALSE(utils::ToJsonFile(1, "invalid_dir/file.json"));

    // Reading non-existent file
    EXPECT_THROW(utils::FromJsonFile<int>("ghost.json"), std::runtime_error);
    EXPECT_EQ(utils::ReadJsonFileAsString("ghost.json"), "");
}