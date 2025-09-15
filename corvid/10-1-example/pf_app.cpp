#include "gridpack/include/gridpack.hpp"
#include "pf_app.hpp"
#include "pf_factory.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <complex>
#include <memory>
#include <cmath>

namespace
{
constexpr double PI = 3.14159265358979323846;
using PFNetwork = gridpack::powerflow::PFNetwork;

struct PFState
{
    bool inited = false;
    boost::shared_ptr<PFNetwork> net;
    gridpack::utility::Configuration *cfg = nullptr;
    gridpack::utility::Configuration::CursorPtr cursor;
    int bus_idx = -1;
    double baseMVA = 100.0;

    std::unique_ptr<gridpack::powerflow::PFFactory> fact;
    std::unique_ptr<gridpack::mapper::BusVectorMap<PFNetwork>> vMap;
    std::unique_ptr<gridpack::mapper::FullMatrixMap<PFNetwork>> jMap;
    boost::shared_ptr<gridpack::math::Vector> PQ, X;
    boost::shared_ptr<gridpack::math::Matrix> J;
    std::unique_ptr<gridpack::math::LinearSolver> solver;
};

inline PFState &state()
{
    static PFState S;
    return S;
}
} // namespace

gridpack::powerflow::PFApp::PFApp(void) {}
gridpack::powerflow::PFApp::~PFApp(void) {}

enum Parser
{
    PTI23,
    PTI33
};

void gridpack::powerflow::PFApp::execute(int argc, char **argv,
                                         std::complex<double> &Vout,       // output: bus-2 phasor
                                         const std::complex<double> &Sinj) // input: aggregated S in pu
{
    gridpack::parallel::Communicator world;

    if (!state().inited)
    {
        state().net.reset(new PFNetwork(world));

        state().cfg = gridpack::utility::Configuration::configuration();
        state().cfg->enableLogging(&std::cout);

        bool opened = false;
        if (argc >= 2 && argv[1] != nullptr)
            opened = state().cfg->open(argv[1], world);
        else
            opened = state().cfg->open("118.xml", world);
        if (!opened)
        {
            if (world.rank() == 0) std::cerr << "PFApp: open config failed\n";
            return;
        }

        state().cursor = state().cfg->getCursor("Configuration.Powerflow");
        state().baseMVA = state().cursor->get("baseMVA", 100.0);

        std::string filename;
        int filetype = PTI23;
        if (!state().cursor->get("networkConfiguration", &filename))
        {
            if (state().cursor->get("networkConfiguration_v33", &filename))
                filetype = PTI33;
            else
            {
                if (world.rank() == 0) std::cerr << "No networkConfiguration\n";
                return;
            }
        }
        const double phaseShiftSign = state().cursor->get("phaseShiftSign", 1.0);

        if (filetype == PTI23)
        {
            if (world.rank() == 0) std::cout << "Using V23 parser for " << filename << "\n";
            gridpack::parser::PTI23_parser<PFNetwork> parser(state().net);
            parser.parse(filename.c_str());
            if (phaseShiftSign == -1.0) parser.changePhaseShiftSign();
        }
        else
        {
            if (world.rank() == 0) std::cout << "Using V33 parser for " << filename << "\n";
            gridpack::parser::PTI33_parser<PFNetwork> parser(state().net);
            parser.parse(filename.c_str());
            if (phaseShiftSign == -1.0) parser.changePhaseShiftSign();
        }

        // Locate original bus 2
        state().bus_idx = -1;
        for (int i = 0; i < state().net->numBuses(); ++i)
        {
            if (state().net->getOriginalBusIndex(i) == 2)
            {
                state().bus_idx = i;
                break;
            }
        }
        if (state().bus_idx == -1)
        {
            if (world.rank() == 0) std::cerr << "Bus 2 not found\n";
            return;
        }

        // One-time build
        state().net->partition();

        state().fact.reset(new gridpack::powerflow::PFFactory(state().net));
        state().fact->load();
        state().fact->setComponents();
        state().fact->setExchange();
        state().net->initBusUpdate();
        state().fact->setYBus();
        state().fact->setSBus();

        // Solver/maps
        state().fact->setMode(RHS);
        state().vMap.reset(new gridpack::mapper::BusVectorMap<PFNetwork>(state().net));
        state().PQ = state().vMap->mapToVector();

        state().fact->setMode(Jacobian);
        state().jMap.reset(new gridpack::mapper::FullMatrixMap<PFNetwork>(state().net));
        state().J = state().jMap->mapToMatrix();

        state().X.reset(state().PQ->clone());
        state().solver.reset(new gridpack::math::LinearSolver(*state().J));
        state().solver->configure(state().cursor);

        state().inited = true;
    }

    // Apply S (pu in MW/Mvar) and solve
    const double P_MW = Sinj.real() * state().baseMVA;
    const double Q_Mvar = Sinj.imag() * state().baseMVA;
    state().net->getBusData(state().bus_idx)->setValue(LOAD_PL, P_MW, 0);
    state().net->getBusData(state().bus_idx)->setValue(LOAD_QL, Q_Mvar, 0);

    const double tolerance = state().cursor->get("tolerance", 1.0e-6);
    const int maxIt = state().cursor->get("maxIteration", 50);

    state().fact->setMode(RHS);
    state().vMap->mapToVector(state().PQ);

    state().fact->setMode(Jacobian);
    state().jMap->mapToMatrix(state().J);

    state().X->zero();
    state().solver->solve(*state().PQ, *state().X);
    auto tol = state().PQ->normInfinity();

    int it = 0;
    while (std::real(tol) > tolerance && it < maxIt)
    {
        state().fact->setMode(RHS);
        state().vMap->mapToBus(state().X);
        state().net->updateBuses();
        state().vMap->mapToVector(state().PQ);

        state().fact->setMode(Jacobian);
        state().jMap->mapToMatrix(state().J);

        state().X->zero();
        state().solver->solve(*state().PQ, *state().X);
        tol = state().PQ->normInfinity();
        ++it;
    }

    // Push solution and return bus-2 voltage
    state().fact->setMode(RHS);
    state().vMap->mapToBus(state().X);
    state().net->updateBuses();

    std::string phase = "A";
    if (argc >= 3 && argv[2] != nullptr) phase = argv[2];

    const double vmag = state().net->getBus(state().bus_idx)->getVoltage();
    const double vang_deg = state().net->getBus(state().bus_idx)->getPhase(); // deg
    Vout = std::polar(vmag, vang_deg * PI / 180.0);

    if (world.rank() == 0)
    {
        const std::string filename_out = "bus_voltages_phase" + phase + ".csv";
        std::ofstream outFile(filename_out);
        outFile << "Original Bus Number,Voltage Magnitude (pu),Voltage Angle (deg)\n";
        for (int i = 0; i < state().net->numBuses(); ++i)
        {
            outFile << state().net->getOriginalBusIndex(i) << "," << state().net->getBus(i)->getVoltage() << ","
                    << state().net->getBus(i)->getPhase() << "\n";
        }
        outFile.close();
        std::cout << "Bus voltages written to " << filename_out << "\n";
    }
}

void gridpack::powerflow::PFApp::finalize()
{
    auto &state() = state();
    state().solver.reset();
    state().J.reset();
    state().X.reset();
    state().PQ.reset();
    state().jMap.reset();
    state().vMap.reset();
    state().fact.reset();
    state().net.reset();
    state().cfg = nullptr;
    state().bus_idx = -1;
    state().inited = false;
}
