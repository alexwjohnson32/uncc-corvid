#include <cstdlib>
#include <optional>
#include <string>
#include <boost/json.hpp>
#include <iostream>
#include <sstream>
#include <memory>
#include <filesystem>

#include "model.hpp"
#include "orchestrator.hpp"
#include "JsonTemplates.hpp"

bool DeployCosim(const std::vector<std::unique_ptr<orchestrator::IModel>> &models)
{
    for (const std::unique_ptr<orchestrator::IModel> &model : models)
    {
        if (!model->DeployExecutables())
        {
            return false;
        }

        if (!model->DeployResources())
        {
            return false;
        }
    }
    return true;
}

std::optional<std::filesystem::path> GenerateJson(const std::filesystem::path &deploy_dir,
                                     const std::vector<std::unique_ptr<orchestrator::IModel>> &models)
{
    std::optional<std::filesystem::path> json_path{};

    orchestrator::HelicsRunner runner;
    runner.name = "IEEE_8500node_IEEE118_gpk_CoSimulation-HELICSRunner";
    runner.broker.core_type = "zmq";
    runner.broker.init_string = "--federates=2 --localport=23500";
    runner.logging_path = "";

    for (const std::unique_ptr<orchestrator::IModel> &model : models)
    {
        orchestrator::HelicsRunner::Federate federate;
        federate.directory = model->GetExecutableDirectory();
        federate.exec = model->GetExecString();
        federate.host = model->GetHost();
        federate.name = model->GetName();
        runner.federates.push_back(federate);
    }

    std::filesystem::path file_path(deploy_dir);
    file_path /= "runnable_cosim.json";

    if (json_templates::ToJsonFile(runner, file_path.generic_string()))
    {
        json_path = file_path;
    }

    return json_path;
}

int RunHelics(const std::filesystem::path &helics_run_file)
{
    std::filesystem::path helics_run_dir = helics_run_file.parent_path();

    std::stringstream command_builder;
    command_builder << "cd " << helics_run_dir.generic_string() << " && "
                    << "helics run --path=" << helics_run_file.filename();
    std::string command_string = command_builder.str();

    std::cout << "Attempting to run command: '" << command_string << "'" << std::endl;
    return std::system(command_string.c_str());
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Please provide a Deploy directory." << std::endl;
        return 0;
    }

    std::filesystem::path deploy_dir(argv[1]);
    if (!std::filesystem::exists(deploy_dir))
    {
        std::cout << "Provided directory `" << deploy_dir.generic_string() << "' does not exist." << std::endl;
        return 0;
    }

    std::vector<std::unique_ptr<orchestrator::IModel>> models;
    models.push_back(std::make_unique<orchestrator::GridPack118BusModel>(deploy_dir));
    models.push_back(std::make_unique<orchestrator::GridLabD8500NodeModel>(deploy_dir));

    if (!DeployCosim(models))
    {
        std::cout << "Could not deploy cosim to directory `" << deploy_dir.generic_string() << "'." << std::endl;
        return 0;
    }

    std::optional<std::filesystem::path> json_file = GenerateJson(deploy_dir, models);
    if (!json_file.has_value())
    {
        std::cout << "Could not generate json file at '" << deploy_dir.generic_string() << "'." << std::endl;
        return 0;
    }

    return RunHelics(json_file.value());
}