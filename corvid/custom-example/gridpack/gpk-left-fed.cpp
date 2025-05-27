#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <complex>
#include <helics/application_api/ValueFederate.hpp>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/Inputs.hpp>

// GridPACK inludes
#include "mpi.h"
#include <ga.h>
#include <macdecls.h>
#include "gridpack/include/gridpack.hpp"
#include "pf_app.hpp"

// Local includes
#include "GpkRunner.hpp"

int main(int argc, char **argv)
{
    // Prepare GridPACK Environment
    gridpack::Environment env(argc, argv);
    gridpack::math::Initialize(&argc, &argv);

    // If we pass an empty file, the PFApp has a default file to use (dont do this for real)
    std::string config_input_file = "";
    if (argc >= 2 && argv[1] != NULL)
    {
        config_input_file = std::string(argv[1]);
    }
    gridpack::powerflow::PFApp app(config_input_file);

    if (!app.CanReadConfigFile())
    {
        std::cout << "Cannot read GridPACK config, exiting.";
        return 0;
    }

    const std::string gpk_left(
        "/beegfs/users/lwilliamson/repos/uncc_root/uncc-corvid/corvid/custom-example/gridpack/gpk-left-fed");
    const std::string out_file_path(
        "/beegfs/users/lwilliamson/repos/uncc_root/uncc-corvid/corvid/custom-example/outputs/gpk.csv");

    // Simulation Initialization
    corvid::GpkRunner runner(gpk_left, 1.0, out_file_path);
    runner.PrepareForSimulation(std::complex<double>(69000.0, 0.0));

    // Entering simulation loop
    double total_interval = 10.0;
    double grantedtime = 0.0;
    while (grantedtime < total_interval)
    {
        grantedtime = runner.AdvanceSimulation(grantedtime, app);
    }

    // Finalize the federate to clean up and disconnect from the HELCIS core
    runner.FinalizeSimulation();

    // Terminate GridPACK Math Libraries
    gridpack::math::Finalize();

    return 0;
}