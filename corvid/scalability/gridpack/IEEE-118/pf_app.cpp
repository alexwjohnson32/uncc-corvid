#include "gridpack/include/gridpack.hpp"
#include "pf_app.hpp"
#include "pf_factory.hpp"
#include <iostream>
#include <fstream>

#include "boost/smart_ptr/shared_ptr.hpp"

gridpack::powerflow::PFApp::PFApp(void) {}
gridpack::powerflow::PFApp::~PFApp(void) {}

enum Parser
{
    PTI23,
    PTI33
};

std::optional<std::complex<double>>
gridpack::powerflow::PFApp::ComputeVoltageCurrent(const std::string &config_file, int target_bus_id,
                                                  const std::string &phase_name, const std::complex<double> &Sa) const
{
    std::optional<std::complex<double>> vc;

    gridpack::parallel::Communicator world;
    boost::shared_ptr<PFNetwork> network(new PFNetwork(world));

    gridpack::utility::Configuration *config = gridpack::utility::Configuration::configuration();
    config->enableLogging(&std::cout);
    bool opened;
    if (!config_file.empty())
    {
        opened = config->open(config_file, world);
    }
    else
    {
        opened = config->open("118.xml", world);
    }
    if (!opened) return vc;

    gridpack::utility::Configuration::CursorPtr cursor;
    cursor = config->getCursor("Configuration.Powerflow");
    std::string filename;
    int filetype = PTI23;
    if (!cursor->get("networkConfiguration", &filename))
    {
        if (cursor->get("networkConfiguration_v33", &filename))
        {
            filetype = PTI33;
        }
        else
        {
            printf("No network configuration file specified\n");
            return vc;
        }
    }

    double tolerance = cursor->get("tolerance", 1.0e-6);
    int max_iteration = cursor->get("maxIteration", 50);
    double phaseShiftSign = cursor->get("phaseShiftSign", 1.0);

    if (world.rank() == 0) printf("Network filename: (%s)\n", filename.c_str());
    if (filetype == PTI23)
    {
        if (world.rank() == 0) printf("Using V23 parser\n");
        gridpack::parser::PTI23_parser<PFNetwork> parser(network);
        parser.parse(filename.c_str());
        if (phaseShiftSign == -1.0) parser.changePhaseShiftSign();
    }
    else if (filetype == PTI33)
    {
        if (world.rank() == 0) printf("Using V33 parser\n");
        gridpack::parser::PTI33_parser<PFNetwork> parser(network);
        parser.parse(filename.c_str());
        if (phaseShiftSign == -1.0) parser.changePhaseShiftSign();
    }

    // Inject complex power Sa into the target bus
    int matched_index = -1;
    for (int i = 0; i < network->numBuses(); ++i)
    {
        if (network->getOriginalBusIndex(i) == target_bus_id)
        {
            matched_index = i;
            break;
        }
    }

    if (matched_index == -1)
    {
        std::cerr << "Original bus number " << target_bus_id << " not found.\n";
        return vc;
    }

    network->getBusData(matched_index)->setValue(LOAD_PL, Sa.real(), 0);
    network->getBusData(matched_index)->setValue(LOAD_QL, Sa.imag(), 0);

    network->partition();

    gridpack::powerflow::PFFactory factory(network);
    factory.load();
    factory.setComponents();
    factory.setExchange();
    network->initBusUpdate();
    factory.setYBus();
    factory.setSBus();

    factory.setMode(RHS);
    gridpack::mapper::BusVectorMap<PFNetwork> pf_network_bus_map(network);
    boost::shared_ptr<gridpack::math::Vector> PQ = pf_network_bus_map.mapToVector();

    factory.setMode(Jacobian);
    gridpack::mapper::FullMatrixMap<PFNetwork> jMap(network);
    boost::shared_ptr<gridpack::math::Matrix> J = jMap.mapToMatrix();

    boost::shared_ptr<gridpack::math::Vector> X(PQ->clone());
    gridpack::math::LinearSolver solver(*J);
    solver.configure(cursor);

    ComplexType tol = 2.0 * tolerance;
    int iter = 0;
    X->zero();
    solver.solve(*PQ, *X);
    tol = PQ->normInfinity();

    while (real(tol) > tolerance && iter < max_iteration)
    {
        factory.setMode(RHS);
        pf_network_bus_map.mapToBus(X);
        network->updateBuses();
        pf_network_bus_map.mapToVector(PQ);
        factory.setMode(Jacobian);
        jMap.mapToMatrix(J);
        X->zero();
        solver.solve(*PQ, *X);
        tol = PQ->normInfinity();
        iter++;
    }

    factory.setMode(RHS);
    pf_network_bus_map.mapToBus(X);
    network->updateBuses();

    // Output file name logic
    std::string phase = "A"; // default
    if (!phase_name.empty())
    {
        phase = phase_name;
    }
    std::string filename_out = "bus_voltages_phase" + phase + ".csv";

    // Set return voltage
    vc = std::polar(network->getBus(matched_index)->getVoltage(), network->getBus(matched_index)->getPhase());

    // Write voltages for all buses
    if (world.rank() == 0)
    {
        std::ofstream outFile(filename_out);
        outFile << "Original Bus Number,Voltage Magnitude (pu),Voltage Angle (deg)\n";
        for (int i = 0; i < network->numBuses(); ++i)
        {
            int busnum = network->getOriginalBusIndex(i);
            double vmag = network->getBus(i)->getVoltage();
            double vang = network->getBus(i)->getPhase();
            outFile << busnum << "," << vmag << "," << vang << "\n";
        }
        outFile.close();
        std::cout << "Bus voltages written to " << filename_out << "\n";
    }

    return vc;
}
