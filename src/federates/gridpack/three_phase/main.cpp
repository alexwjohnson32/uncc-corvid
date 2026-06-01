#include <iostream>
#include <filesystem>
#include <string>
#include <exception>

#include "inputs.hpp"
#include "federate.hpp"
#include "common/utils/json_templates.hpp"

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

    three_phase::ThreePhaseInput input = common::utils::FromJsonFile<three_phase::ThreePhaseInput>(json_file);
    three_phase::ThreePhaseFederate fed;

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
