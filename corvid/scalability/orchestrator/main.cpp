#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <sstream>

struct CustomObject
{
    std::string m_member;
};

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, CustomObject data)
{
    json_value = { { "member", data.m_member } };
}

CustomObject tag_invoke(boost::json::value_to_tag<CustomObject>, boost::json::value const &json_value)
{
    CustomObject data;
    const boost::json::object &obj = json_value.as_object();
    const boost::json::value *found_value = obj.if_contains("member");
    if (found_value)
    {
        data.m_member = boost::json::value_to<std::string>(*found_value);
    }
    else
    {
        data.m_member = "not found";
    }

    return data;
}

void TestJson(const std::string &input_value)
{
    std::stringstream ss;
    CustomObject data { input_value };
    ss << boost::json::serialize(boost::json::value_from(data));

    CustomObject copied_data = boost::json::value_to<CustomObject>(boost::json::parse(ss.str()));
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