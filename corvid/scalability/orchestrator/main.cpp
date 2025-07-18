#include <cstdlib>
#include <string>
#include <boost/json.hpp>
#include <iostream>
#include <sstream>

#include "orchestrator.hpp"
#include "JsonTemplates.hpp"

orchestrator::HelicsRunner GetRunner()
{
    orchestrator::HelicsRunner runner;
    runner.name = "IEEE_8500node_IEEE118_gpk_CoSimulation-HELICSRunner";
    runner.broker.core_type = "zmq";
    runner.broker.init_string = "--federates=2 --localport=23500";
    runner.logging_path = "";

    orchestrator::HelicsRunner::Federate gridlab_fed;
    gridlab_fed.directory = ".";
    gridlab_fed.exec = "gridlabd.sh gridlabd/IEEE_8500node.glm";
    gridlab_fed.host = "localhost";
    gridlab_fed.name = "gld_fed";
    runner.federates.push_back(gridlab_fed);

    orchestrator::HelicsRunner::Federate gridpack_fed;
    gridpack_fed.directory = ".";
    gridpack_fed.exec = "./gridpack/IEEE-118/powerflow_ex.x";
    gridpack_fed.host = "localhost";
    gridpack_fed.name = "gpk_fed";
    runner.federates.push_back(gridpack_fed);

    return runner;
}

int main(int argc, char **argv)
{
    orchestrator::HelicsRunner runner = GetRunner();
    const std::string runnable_path = "runnable_cosim.json";

    if (json_templates::ToJsonFile(runner, runnable_path))
    {
        std::stringstream system_command;
        system_command << "helics run --path=" << runnable_path;

        std::cout << "Attempting to run command: '" << system_command.str() << "'" << std::endl;
        return std::system(system_command.str().c_str());
    }
    else
    {
        std::cout << "Unable to run because json file could not be written to " << std::getenv("PWD") << "/"
                  << runnable_path << std::endl;
        return 0;
    }
}