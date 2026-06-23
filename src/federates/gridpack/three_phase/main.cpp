#include <iostream>
#include <filesystem>
#include <string>
#include <exception>
#include <memory>

#include "inputs.hpp"
#include "federate.hpp"
#include "common/utils/json_templates.hpp"

// GridPACK includes
#include "mpi.h"
#include <ga.h>
#include <macdecls.h>
#include "gridpack/include/gridpack.hpp"

namespace
{
std::shared_ptr<helics::ValueFederate> GetGridpackFederate(const three_phase::ThreePhaseInput &pf_input)
{
    // Create a FederateInfo object
    helics::FederateInfo fi;
    fi.loadInfoFromJson(pf_input.fed_info_json);

    return std::make_shared<helics::ValueFederate>(pf_input.federate_name, fi);
}
} // namespace

int main(int argc, char **argv)
{
    gridpack::Environment env(argc, argv);
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
    std::shared_ptr<helics::ValueFederate> fed_ptr = GetGridpackFederate(input);

    try
    {
        three_phase::ThreePhaseFederate fed;
        fed.Initialize(input, fed_ptr);
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

    gridpack::math::Finalize();
    fed_ptr->finalize();

    return ret_val;
}
