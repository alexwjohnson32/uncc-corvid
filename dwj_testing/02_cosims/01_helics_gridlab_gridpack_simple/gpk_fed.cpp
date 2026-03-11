#include <helics/application_api/ValueFederate.hpp>
#include <helics/application_api/Subscriptions.hpp>
#include <helics/application_api/Publications.hpp>
#include <mpi.h>
#include <iostream>
#include <string>
#include <complex> // Required for GridLAB-D complex types

int main(int argc, char** argv) {
    // MPI initialization if needed for GridPACK later
    MPI_Init(&argc, &argv);
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    try {
        helics::FederateInfo fi;
        fi.coreType = helics::CoreType::ZMQ;
        // Use the default port 23404 to match your recent broker setup
        fi.coreInitString = "--broker=localhost --port=23404";
        fi.setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
        
        helics::ValueFederate vFed("gridpack_fed", fi);
        if (rank == 0) std::cout << "GridPACK Federate Created" << std::endl;

        // --- 1. Register Subscriptions and Publications ---
        // We subscribe to the complex load from GridLAB-D
        auto& sub = vFed.registerSubscription("gridlabd/total_load", "complex");
        
        // We publish a double voltage back to GridLAB-D
        auto& pub = vFed.registerGlobalPublication("gridpack/bus_voltage", "double");

        // --- 2. Enter Execution Mode ---
        vFed.enterExecutingMode();
        if (rank == 0) std::cout << "Entered Execution Mode" << std::endl;

        double current_time = 0.0;
        double mock_voltage = 7200.0; // Starting voltage

        // --- 3. Main Co-simulation Loop ---
        while (current_time < 60.0) {
            current_time = vFed.requestTime(current_time + 1.0);

            if (vFed.isUpdated(sub)) {
                // Retrieve complex value from GridLAB-D
                std::complex<double> load = sub.getValue<std::complex<double>>();
                
                if (rank == 0) {
                    std::cout << "[T=" << current_time << "] Received Load: " 
                              << load.real() << " W" << std::endl;

                    // Simulate some "GridPACK" math: voltage drops as load increases
                    // (Simplified: subtract 1V for every 1000W of real power)
                    mock_voltage = 7200.0 - (load.real() / 1000.0);

                    // Publish the result back to GridLAB-D
                    pub.publish(mock_voltage);
                    std::cout << "[T=" << current_time << "] Sent Voltage: " 
                              << mock_voltage << " V" << std::endl;
                }
            }
        }

        vFed.finalize();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    MPI_Finalize();
    return 0;
}