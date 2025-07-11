#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <iostream>

int main(int argc, char **argv)
{
    boost::filesystem::path p("");

    std::vector<std::string> vec;
    vec.push_back(p.generic_string());

    for (auto s : vec)
    {
        std::cout << "vector content: '" << s << "'" << std::endl;
    }

    return 0;
}