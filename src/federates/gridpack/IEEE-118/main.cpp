#include <iostream>
#include <filesystem>
#include <string>
#include <exception>

#include "ieee_118_inputs.hpp"
#include "ieee_118_federate.hpp"
#include "json_templates.hpp"

int main(int argc, char **argv)
{
    int ret_val = EXIT_FAILURE;

    if (argc < 2)
    {
        std::cerr << "Missing JSON: No json file provided in execution call.\n";
        return ret_val;
    }

    const std::filesystem::path cwd = std::filesystem::current_path();
    const std::string json_file = std::string(argv[1]);
    const std::filesystem::path json_path = json_file;
    if (!std::filesystem::exists(json_path))
    {
        std::cerr << "Missing JSON: " << json_path << " (expected in " << cwd << ")\n";
        return ret_val;
    }

    ieee_118::IEEE118Input input = common::utils::FromJsonFile<ieee_118::IEEE118Input>(json_file);
    ieee_118::IEEE118Federate fed;

    try
    {
        fed.Initialize(input, { argc, argv });
        fed.Run();

        ret_val = EXIT_SUCCESS;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown Exception: An object that does not inherit std::exception was thrown!" << std::endl;
    }

    fed.Close();

    return ret_val;
}
