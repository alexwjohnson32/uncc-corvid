#include <cstdlib>
#include <string>
#include <boost/json.hpp>
#include <iostream>

#include "orchestrator.hpp"
#include "JsonTemplates.hpp"

orchestrator::HelicsRunner GetRunner()
{
    orchestrator::HelicsRunner runner;
    runner.name = "example_corvid_cosim";
    runner.broker = true;
    runner.logging_path = "";

    orchestrator::HelicsRunner::Federate fed1;
    fed1.directory = ".";
    fed1.exec = "gridlabd gridlabd/oneline-right-w-gpk.glm";
    fed1.host = "localhost";
    fed1.name = "gridlabd_right_federate";
    runner.federates.push_back(fed1);

    orchestrator::HelicsRunner::Federate fed2;
    fed2.directory = ".";
    fed2.exec = "gridpack/build/gpk-left-fed.x";
    fed2.host = "localhost";
    fed2.name = "gridpack_left_federate";
    runner.federates.push_back(fed2);

    return runner;
}

void TestJson(orchestrator::HelicsRunner input)
{
    std::cout << "original object:\n" << json_templates::ToJsonString(input) << std::endl;
    input.name = "replaced name";
    auto copied_runner =
        json_templates::FromJsonString<orchestrator::HelicsRunner>(json_templates::ToJsonString(input));
    std::cout << "\ncopied object:\n" << json_templates::ToJsonString(copied_runner) << std::endl;
}

int main(int argc, char **argv)
{
    orchestrator::HelicsRunner runner = GetRunner();
    TestJson(runner);

    const std::string runnable_path = "runnable_cosim.json";
    bool written = json_templates::ToJsonFile(runner, runnable_path);

    if (written)
    {
        std::cout << "Json file written to " << std::getenv("PWD") << "/" << runnable_path << std::endl;
    }
    else
    {
        std::cout << "Json file could not be written to " << std::getenv("PWD") << "/" << runnable_path << std::endl;
    }

    return 0;
}