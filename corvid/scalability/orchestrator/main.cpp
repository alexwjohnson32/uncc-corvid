#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <sstream>

#include "orchestrator.hpp"

void TestJson(const std::string &input_value)
{
    std::stringstream ss;
    orchestrator::CustomObject data{ input_value };
    ss << boost::json::serialize(boost::json::value_from(data));

    orchestrator::CustomObject copied_data =
        boost::json::value_to<orchestrator::CustomObject>(boost::json::parse(ss.str()));
    data.m_member = "";

    std::cout << "Original Object: '" << data.m_member << "'" << std::endl;
    std::cout << "Copied Object: '" << copied_data.m_member << "'" << std::endl;
}

int main(int argc, char **argv)
{
    boost::filesystem::path p("");

    std::vector<std::string> vec;
    vec.push_back(p.generic_string());

    for (auto s : vec)
    {
        std::cout << "vector content: '" << s << "'" << std::endl;
    }

    TestJson("Testing Input");

    return 0;
}