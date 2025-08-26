#include <cstdlib>
#include <optional>
#include <string>
#include <boost/json.hpp>
#include <iostream>
#include <sstream>
#include <memory>
#include <filesystem>
#include <vector>

#include "model.hpp"
#include "orchestrator.hpp"
#include "JsonTemplates.hpp"

bool DeployCosim(const std::vector<std::unique_ptr<orchestrator::IModel>> &models)
{
    for (const std::unique_ptr<orchestrator::IModel> &model : models)
    {
        if (!model->DeployExecutables()) return false;
        if (!model->DeployResources()) return false;
    }
    return true;
}

void PrintModelAndFederate(const orchestrator::HelicsRunner::Federate &federate,
                           const std::unique_ptr<orchestrator::IModel> &model)
{
    std::cout << "Executable Dir:\nmodel:\t" << model->GetExecutableDirectory() << "\nfederate:\t" << federate.directory
              << std::endl;
    std::cout << "Executable:\nmodel:\t" << model->GetExecString() << "\nfederate:\t" << federate.exec << std::endl;
    std::cout << "Host:\nmodel:\t" << model->GetHost() << "\nfederate:\t" << federate.host << std::endl;
    std::cout << "Name:\nmodel:\t" << model->GetName() << "\nfederate:\t" << federate.directory << std::endl;
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

    if (json_templates::ToJsonFile(runner, file_path.generic_string())) json_path = file_path;

    return json_path;
}

int RunHelics(const std::filesystem::path &helics_run_file)
{
    std::filesystem::path helics_run_dir = helics_run_file.parent_path();

    std::stringstream command_builder;
    command_builder << "cd \"" << helics_run_dir.string() << "\" && "
                    << "helics run --path=\"" << helics_run_file.filename().string() << "\"";
    std::string command_string = command_builder.str();

    std::cout << "Attempting to run command: '" << command_string << "'" << std::endl;
    return std::system(command_string.c_str());
}

int main(int argc, char **argv)
{
    // Hard-coded deploy dir; you will create it in the shell script
    std::filesystem::path deploy_dir = "deploy";

    // Single optional flag: --existing (leave deploy/ contents alone)
    bool existing = false;
    if (argc >= 2)
    {
        std::string arg = argv[1];
        if (arg == "--existing")
        {
            existing = true;
        }
        else
        {
            std::cout << "Usage: " << argv[0] << " [--existing]\n";
            return 1;
        }
    }

    // Require deploy/ exists (you said the shell script will create it)
    if (!std::filesystem::exists(deploy_dir))
    {
        std::cout << "Deploy dir `" << deploy_dir.generic_string() << "` does not exist. Create it before running.\n";
        return 1;
    }

    // Build models (hard-coded deploy path)
    std::vector<std::unique_ptr<orchestrator::IModel>> models;
    models.push_back(std::make_unique<orchestrator::GridPack118BusModel>(deploy_dir.string()));
    models.push_back(std::make_unique<orchestrator::GridLabD8500NodeModel>(deploy_dir.string()));

    // In --existing mode, do not create/copy/write anything inside deploy/
    for (auto &m : models) m->SetAllowCreateDirs(!existing);

    if (existing)
    {
        std::cout << "Existing mode: leaving deploy/ untouched (no copies, no writes).\n";
        std::filesystem::path prebuilt = deploy_dir / "runnable_cosim.json";
        if (!std::filesystem::exists(prebuilt))
        {
            std::cout << "Expected existing run file at '" << prebuilt.generic_string() << "' but it was not found.\n";
            return 1;
        }
        return RunHelics(prebuilt);
    }

    // Normal path: perform deployment and generate the run file
    if (!DeployCosim(models))
    {
        std::cout << "Could not deploy cosim to directory `" << deploy_dir.generic_string() << "'." << std::endl;
        return 1;
    }

    std::optional<std::filesystem::path> json_file = GenerateJson(deploy_dir, models);
    if (!json_file.has_value())
    {
        std::cout << "Could not generate json file at '" << deploy_dir.generic_string() << "'." << std::endl;
        return 1;
    }

    return RunHelics(json_file.value());
}
