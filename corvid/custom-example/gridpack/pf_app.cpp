/*
 *     Copyright (c) 2013 Battelle Memorial Institute
 *     Licensed under modified BSD License. A copy of this license can be found
 *     in the LICENSE file in the top level directory of this distribution.
 */
// -------------------------------------------------------------
/**
 * @file   pf_app.hpp
 * @author Bruce Palmer
 * @date   2014-01-28 11:33:42 d3g096
 *
 * @brief
 *
 * I don't know if this is necessary, I have mostly copied a copy so here we are.
 */
// -------------------------------------------------------------

#include "gridpack/include/gridpack.hpp"
#include "pf_app.hpp"
#include <iostream>
#include <cstdlib>
#include <string>
#include <cmath>

// Calling program for powerflow application

gridpack::powerflow::PFApp::PFApp(const std::string &alt_config_path)
    : m_config_path(gridpack::powerflow::PFApp::CompiledConfigPath())
{
    // If a file is not specified when invoking the
    // executable, assume the input file is called "input.xml"
    if (!alt_config_path.empty())
    {
        m_config_path = alt_config_path;
    }
}

boost::shared_ptr<gridpack::utility::Configuration> gridpack::powerflow::PFApp::GetConfig(gridpack::parallel::Communicator &world) const
{
    boost::shared_ptr<gridpack::utility::Configuration> config(gridpack::utility::Configuration::configuration());
    config->enableLogging(&std::cout);

    if (!config->open(m_config_path, world))
    {
        std::string pwd = "";
        const char *env_var = std::getenv("PWD");
        if (env_var != nullptr) pwd = std::string(env_var);

        std::cout << "RETURNING: Could not find file: '" << m_config_path << "'. \n\tWorking Dir: " << pwd << std::endl;
        config = nullptr;
    }

    return config;
}

std::string gridpack::powerflow::PFApp::ParseNetworkConfig(boost::shared_ptr<PFNetwork> network,
                                                           gridpack::utility::Configuration::CursorPtr cursor, int rank) const
{
    // Different files use different conventions for the phase shift sign.
    // Allow users to change sign to correspond to the convention used
    // in this application.
    double phase_shift_sign = cursor->get("phaseShiftSign", 1.0);

    // If networkConfiguration field found in input, assume that file
    // is PSS/E version 23 format, otherwise if networkConfiguration_v33
    // field found in input, assume file is version 33 format. If neither
    // field is found, then no configuration file is specified so return.
    // The rank() function on the communicator is used to determine the processor ID
    std::string network_config_file = "";
    if (cursor->get("networkConfiguration", &network_config_file))
    {
        // PTI23 parser
        if (rank == 0)
        {
            std::cout << "Using V23 parser" << std::endl;
            std::cout << "Network filename: " << network_config_file << std::endl;
        }

        gridpack::parser::PTI23_parser<PFNetwork> parser(network);
        parser.parse(network_config_file.c_str());

        if (phase_shift_sign == -1.0)
        {
            parser.changePhaseShiftSign();
        }
    }
    else if (cursor->get("networkConfiguration_v33", &network_config_file))
    {
        // PTI33 parser
        if (rank == 0)
        {
            std::cout << "Using V33 parser" << std::endl;
            std::cout << "Network filename: " << network_config_file << std::endl;
        }

        gridpack::parser::PTI33_parser<PFNetwork> parser(network);
        parser.parse(network_config_file.c_str());
        if (phase_shift_sign == -1.0)
        {
            parser.changePhaseShiftSign();
        }
    }
    else
    {
        // Unknown parser
        std::cout << "No network configuration file specified" << std::endl;
        network_config_file = "";
    }

    return network_config_file;
}

std::complex<double> gridpack::powerflow::PFApp::ComputeVc(const std::complex<double> &Sa) const
{
    // Define a communicator on the group of all processors (the world group)
    // and create and instance of a power flow network
    gridpack::parallel::Communicator world;
    boost::shared_ptr<PFNetwork> network(new PFNetwork(world));

    boost::shared_ptr<gridpack::utility::Configuration> config = GetConfig(world);
    if (config == nullptr)
    {
        return std::complex<double>(std::nan(""), std::nan(""));
    }

    // Find the Configuration.Powerflow block within the input file
    // and set cursor pointer to that block
    gridpack::utility::Configuration::CursorPtr cursor;
    cursor = config->getCursor("Configuration.Powerflow");

    // The rank() function on the communicator is used to determine the processor ID
    // If the network config is empty, none could be parsed.
    std::string network_config_file = ParseNetworkConfig(network, cursor, world.rank());
    if (network_config_file.empty())
    {
        return std::complex<double>(std::nan(""), std::nan(""));
    }

    // Set convergence and iteration parameters from input file. If
    // tolerance and maxIteration fields not found, then use defaults
    double tolerance = cursor->get("tolerance", 1.0e-6);
    int max_iteration = cursor->get("maxIteration", 50);

    network->getBusData(1)->setValue(LOAD_PL, Sa.real(), 0);
    network->getBusData(1)->setValue(LOAD_QL, Sa.imag(), 0);

    // Partition network between processors
    network->partition();

    // Echo number of buses and branches to standard out. This message prints from
    // each processor. These numbers may be different for different processors.
    std::cout << "Process: " << world.rank() << " NBUS: " << network->numBuses()
              << " NBRANCH: " << network->numBranches() << std::endl;

    // Create serial IO object to export data from buses
    gridpack::serial_io::SerialBusIO<PFNetwork> busIO(8192, network);
    char ioBuf[128];

    // Echo convergence parameters to standard out. The use of the header
    // method in the SerialBusIO class guarantees that the message is only
    // written once to standard out.
    sprintf(ioBuf, "\nMaximum number of iterations: %d\n", max_iteration);
    busIO.header(ioBuf);
    sprintf(ioBuf, "\nConvergence tolerance: %f\n", tolerance);
    busIO.header(ioBuf);

    // Create factory and call the load method to intialize network components
    // from information in configuration file
    gridpack::powerflow::PFFactory factory(network);
    factory.load();

    // Set network components using factory. This includes defining internal
    // indecis that are used in data exchanges and to manage IO
    factory.setComponents();

    // Set up bus data exchange buffers. The data to be exchanged is defined
    // in the networkd component classes.
    factory.setExchange();

    // Create bus data exchange. Data exchanges between branches are not needed
    // for this calculation
    network->initBusUpdate();

    // Create components Y-matrix
    factory.setYBus();

    // Create components of S vector and print out first iteration count
    // to standard output
    factory.setSBus();
    busIO.header("\nIteration 0\n");

    factory.setMode(RHS);
    gridpack::mapper::BusVectorMap<PFNetwork> vMap(network);
    boost::shared_ptr<gridpack::math::Vector> PQ = vMap.mapToVector();

    // Create Jacobian matrix
    factory.setMode(Jacobian);
    gridpack::mapper::FullMatrixMap<PFNetwork> jMap(network);
    boost::shared_ptr<gridpack::math::Matrix> J = jMap.mapToMatrix();

    // Create X (solution) vector by cloning PQ
    boost::shared_ptr<gridpack::math::Vector> X(PQ->clone());

    // <latex> The LinearSolver object solves equations of the form
    // $\overline{\overline{A}}\cdot\overline{X}=\overline{B}$ </latex>
    // Create linear solver and configure it with settings from the
    // input file (inside the LinearSolver block)
    gridpack::math::LinearSolver solver(*J);
    solver.configure(cursor);

    // Set initial value of the tolerance
    ComplexType tol = 2.0 * tolerance;
    int iter = 0;

    // First iteration of the solver to intialize Newton-Raphson loop
    X->zero(); // might not need to do this
    busIO.header("\nCalling solver\n");
    solver.solve(*PQ, *X);

    // <latex> normInfinity evaluates the norm $N=\max_{i}|X_i|$</latex>
    // The Newton-Raphson algorithm evaluates incremental changes to the solution
    // vector. When the algorithm is done, the RHS vector is zero
    tol = PQ->normInfinity();

    while (real(tol) > tolerance && iter < max_iteration)
    {
        // Push current values in X vector back into network components
        // This uses setValues method in PFBus class in order to work
        factory.setMode(RHS);
        vMap.mapToBus(X);

        // Update Jacobian and PQ vector with new values
        vMap.mapToVector(PQ);
        factory.setMode(Jacobian);
        jMap.mapToMatrix(J);

        // Resolve equations (the solver can be resued since the number and
        // location of non-zero elements is the same)
        X->zero(); // might not need to do this
        solver.solve(*PQ, *X);

        // Evaluate norm of residual and print out current iteration to standard out
        tol = PQ->normInfinity();
        sprintf(ioBuf, "\nIteration %d Tol: %12.6e\n", iter + 1, real(tol));
        busIO.header(ioBuf);
        iter++;
    }

    // Push final result back onto buses so we get the right values printed out to outptut
    factory.setMode(RHS);
    vMap.mapToBus(X);

    // Make sure that the ghost buses have up-to-date values before printing out
    // final results (evaluating power flow on branches requires correct values
    // of voltages on all buses)
    network->updateBuses();

    // Write out headers and power flow values for all branches
    gridpack::serial_io::SerialBranchIO<PFNetwork> branchIO(512, network);
    branchIO.header("\n   Branch Power Flow\n");
    branchIO.header("\n        Bus 1       Bus 2   CKT         P"
                    "                    Q\n");

    // Write branch values
    branchIO.write();

    // Write out headers and voltage values for all buses
    busIO.header("\n   Bus Voltages and Phase Angles\n");
    busIO.header("\n   Bus Number      Phase Angle      Voltage Magnitude\n");

    // Write bus values values
    busIO.write();

    return std::polar(network->getBus(1)->getVoltage(), network->getBus(1)->getPhase());
}