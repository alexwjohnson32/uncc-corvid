#include <helics/application_api/ValueFederate.hpp>
#include <helics/application_api/Subscriptions.hpp>
#include <helics/application_api/Publications.hpp>
#include <mpi.h>
#include <iostream>
#include <memory>
#include <complex>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::unique_ptr<helics::ValueFederate> vFed;
    helics::Publication* pub = nullptr;
    helics::Input* sub = nullptr;

    try {
        if (rank == 0) {
            helics::FederateInfo fi;
            fi.coreType = helics::CoreType::MPI; 
            // Explicitly tell HELICS that the Broker is Rank 0 in the MPI_COMM_WORLD
            fi.coreInitString = "--broker_rank=0"; 
            fi.setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
            
            vFed = std::make_unique<helics::ValueFederate>("gridpack_fed", fi);
            sub = &vFed->registerSubscription("gridlabd/total_load", "complex");
            pub = &vFed->registerGlobalPublication("gridpack/bus_voltage", "double");
            
            vFed->enterExecutingMode();
            std::cout << "Rank 0: Connected to HELICS Broker and Entered Execution Mode." << std::endl;
        }

        double current_time = 0.0;
        double mock_voltage = 7200.0;
        std::complex<double> load(0, 0);

        while (current_time < 60.0) {
            if (rank == 0) {
                current_time = vFed->requestTime(current_time + 1.0);
                if (vFed->isUpdated(*sub)) {
                    load = sub->getValue<std::complex<double>>();
                    std::cout << "[T=" << current_time << "] Received Load: " 
                              << load.real() << " W" << std::endl;
                }
            }

            // Sync time and data across all cores
            MPI_Bcast(&current_time, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            MPI_Bcast(&load, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);

            if (rank == 0) {
                mock_voltage = 7200.0 - (load.real() / 1000.0);
                pub->publish(mock_voltage);
                std::cout << "[T=" << current_time << "] Sent Voltage: " 
                          << mock_voltage << " V" << std::endl;
                std::cout.flush(); // Force the text to appear in terminal
            }
            
            MPI_Bcast(&mock_voltage, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        }

        if (rank == 0) {
            std::cout << "Rank 0: Finalizing simulation..." << std::endl;
            vFed->finalize();
        }
    } catch (const std::exception& e) {
        if (rank == 0) std::cerr << "HELICS Error: " << e.what() << std::endl;
    }

    MPI_Finalize();
    return 0;
}