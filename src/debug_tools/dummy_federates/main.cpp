#include "helics_input.hpp"
#include "dummy_federate.hpp"
#include "json_templates.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <filesystem>

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

    common::helics::HelicsInput helics_input = common::utils::FromJsonFile<common::helics::HelicsInput>(json_file);
    dummy::DummyFederate dummy_fed;

    try
    {
        dummy_fed.Initialize(helics_input);
        dummy_fed.Run();

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

    dummy_fed.Close();

    return ret_val;
}