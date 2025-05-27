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

std::complex<double> GetAverage(const std::vector<std::complex<double>> &values)
{
    std::complex<double> total_value(0.0, 0.0);
    int nums = values.size();

    for (const std::complex<double> &value : values)
    {
        total_value += value;
    }

    return total_value / static_cast<double>(nums);;
}

std::string ComplexToString(const std::complex<double> &complex, uint precision)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << "(" << complex.real() << ", " << complex.imag() << ")";
    return ss.str();
}

int main (int argc, char **argv)
{
    // Create a FederateInfo object
    helics::FederateInfo fed_info;

    // Select the core type, set one core per federate
    fed_info.coreType = helics::CoreType::ZMQ;
    fed_info.coreInitString = "--federates=1";

    // Logging in debug mode
    fed_info.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_DEBUG);

    // Set Simulator resolution
    double period = 1.0;
    fed_info.setProperty(HELICS_PROPERTY_TIME_PERIOD, period);

    // Set some flags
    fed_info.setFlagOption(HELICS_FLAG_UNINTERRUPTIBLE, false);
    fed_info.setFlagOption(HELICS_FLAG_TERMINATE_ON_ERROR, true);
    fed_info.setFlagOption(HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE, true);

    // Create Value Federate
    helics::ValueFederate gpk_left("gpk-left-fed", fed_info);
    std::cout << "HELICS GridPACK Federate create successfully." << std::endl;

    // Registering Pubs and Subs
    helics::Publication Vc_id = gpk_left.registerPublication("Vc", "complex", "V");
    std::vector<helics::Input> sa_ids;
    sa_ids.push_back(gpk_left.registerSubscription("gpk_gld_right_fed_1/sa", "VA"));
    sa_ids.push_back(gpk_left.registerSubscription("gpk_gld_right_fed_2/sa", "VA"));

    // File to store simulation signals
    std::ofstream outFile("/beegfs/users/lwilliamson/repos/uncc_root/uncc-corvid/corvid/custom-example/outputs/gpk.csv");

    // Prepare GridPACK Environment
    gridpack::Environment env(argc, argv);
    gridpack::math::Initialize(&argc, &argv);
    gridpack::powerflow::PFApp app;

    // Enter execution mode (this transisitions the federate from initialization to execution)
    gpk_left.enterExecutingMode();
    std::cout << "GridPACK Federeate has entered execution mode." << std::endl;

    // Simulation Initialization
    double total_interval = 10.0;
    double grantedtime = 0.0;

    auto Sa = std::complex<double>(0.0, 0.0);
    auto voltage = std::complex<double>(1.0, 0.0);

    // Publish initial center voltage
    Vc_id.publish(voltage * 69000.0);

    // Entering simulation loop
    while (grantedtime < total_interval)
    {
        // Request time
        grantedtime = gpk_left.requestTime(grantedtime + period);

        // Get Sa's from gridlab-d
        std::vector<std::complex<double>> sa_values;
        for (helics::Input &input : sa_ids)
        {
            std::complex<double> sa = input.getValue<std::complex<double>>() / 1000000.0;
            sa_values.push_back(sa);
            outFile << "Sub Name: " << input.getName() << "\n";
            outFile << "Sub value: " << ComplexToString(sa, 2) << " " << input.getUnits() << "\n";
        }

        Sa = GetAverage(sa_values);

        // pass Sa to GridPACK and get back Vc
        app.execute(argc, argv, voltage, Sa);

        // Log boundary signals
        outFile << "Time (s): " << grantedtime << "\n";
        outFile << "Avg Sa received from Gridlab-D: " << Sa << "\n";
        outFile << "Update Vc by GridPACK:      " << voltage << "\n\n";

        // Publish new Center Bus Voltage
        Vc_id.publish(voltage * 69000.0);
    }

    outFile << "End of Cosimulation.";
    outFile.close();

    // Terminate GridPACK Math Libraries
    gridpack::math::Finalize();

    // Finalize the federate to clean up and disconnect from the HELCIS core
    gpk_left.finalize();
    std::cout << "Federate finalized." << std::endl;

    return 0;
}