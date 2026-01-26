#include <gtest/gtest.h>
#include "local_log_helper.hpp"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <mutex>
#include <thread>
#include <atomic>

namespace fs = std::filesystem;

class LocalLogHelperTest : public ::testing::Test
{
  protected:
    // Track files created during tests to ensure they are cleaned up in TearDown
    std::set<std::string> m_tracked_files;
    std::mutex m_files_mutex;

    /**
     * @brief Generates a unique filename for the current test case.
     * Fixed: Added an atomic counter to ensure uniqueness when called multiple times in one test.
     */
    std::string GetUniqueFileName()
    {
        static std::atomic<int> file_counter{ 0 };
        const ::testing::TestInfo *const test_info = ::testing::UnitTest::GetInstance()->current_test_info();

        std::stringstream ss;
        ss << "log_" << test_info->test_case_name() << "_" << test_info->name() << "_"
           << std::hash<std::thread::id>{}(std::this_thread::get_id()) << "_" << file_counter.fetch_add(1) << ".txt";

        std::string filename = ss.str();

        std::lock_guard<std::mutex> lock(m_files_mutex);
        m_tracked_files.insert(filename);
        return filename;
    }

    void TearDown() override
    {
        std::lock_guard<std::mutex> lock(m_files_mutex);
        for (const auto &file : m_tracked_files)
        {
            if (fs::exists(file))
            {
                fs::remove(file);
            }
        }
        m_tracked_files.clear();
    }

    std::string ReadFileContent(const std::string &path)
    {
        std::ifstream ifs(path);
        if (!ifs.is_open()) return "";
        return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    }
};

TEST_F(LocalLogHelperTest, UniqueFileWriting)
{
    std::string filename = GetUniqueFileName();
    {
        utils::LocalLogHelper logger(filename);
        logger << "Thread-safe data: " << 0xFF;
    }

    EXPECT_EQ(ReadFileContent(filename), "Thread-safe data: 255");
}

TEST_F(LocalLogHelperTest, CallbackIsolation)
{
    std::string filename = GetUniqueFileName();
    std::string captured;

    {
        utils::LocalLogHelper logger(filename);
        logger.SetOnWriteCallback([&](const std::string &msg) { captured = msg; });

        // Verify proxy destruction triggers callback
        logger << "Isolation Test";
    }

    EXPECT_EQ(captured, "Isolation Test");
}

TEST_F(LocalLogHelperTest, MultipleStreamsSameLogger)
{
    std::string filename = GetUniqueFileName();
    std::vector<std::string> results;

    {
        utils::LocalLogHelper logger(filename);
        logger.SetOnWriteCallback([&](const std::string &msg) { results.push_back(msg); });

        // Each line is a separate LogStream proxy
        logger << "Line A";
        logger << "Line B";
    } // Scoped destruction flushes file and ensures all data is written

    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0], "Line A");
    EXPECT_EQ(results[1], "Line B");

    std::string file_data = ReadFileContent(filename);
    EXPECT_NE(file_data.find("Line ALine B"), std::string::npos);
}

TEST_F(LocalLogHelperTest, ManipulatorNewlineTest)
{
    std::string filename = GetUniqueFileName();
    std::string captured;

    {
        utils::LocalLogHelper logger(filename);
        logger.SetOnWriteCallback([&](const std::string &msg) { captured = msg; });

        logger << "Header" << std::endl;
    }

    EXPECT_EQ(captured, "Header\n");
    EXPECT_EQ(ReadFileContent(filename), "Header\n");
}

TEST_F(LocalLogHelperTest, MidSessionFileSwitch)
{
    std::string file1 = GetUniqueFileName();
    std::string file2 = GetUniqueFileName();

    // Verification that filenames are actually distinct
    ASSERT_NE(file1, file2);

    {
        utils::LocalLogHelper logger(file1);
        logger << "Part 1";

        logger.SetOutputFile(file2);
        logger << "Part 2";
    }

    EXPECT_EQ(ReadFileContent(file1), "Part 1");
    EXPECT_EQ(ReadFileContent(file2), "Part 2");
}

TEST_F(LocalLogHelperTest, InvalidPathRecovery)
{
    // A path that is almost certainly invalid (empty directory name)
    std::string invalid_path = "/this/does/not/exist/log.txt";
    std::string captured;

    {
        utils::LocalLogHelper logger(invalid_path);
        EXPECT_FALSE(logger.IsOpen());

        logger.SetOnWriteCallback([&](const std::string &msg) { captured = msg; });

        // The logger should still function for WebSocket/Callbacks even if disk fails
        logger << "Disk failed but I am still here";
    }

    EXPECT_EQ(captured, "Disk failed but I am still here");
}