#include "gridpack/include/gridpack.hpp"
#include "pf_app.hpp"
#include "pf_factory.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <complex>
#include <memory>
#include <cmath>

namespace {
  constexpr double PI = 3.14159265358979323846;
  using PFNetwork = gridpack::powerflow::PFNetwork;

  struct PFState {
    bool inited = false;
    boost::shared_ptr<PFNetwork> net;
    gridpack::utility::Configuration* cfg = nullptr;
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

  inline PFState& state() { static PFState S; return S; }
}

gridpack::powerflow::PFApp::PFApp(void) {}
gridpack::powerflow::PFApp::~PFApp(void) {}

enum Parser { PTI23, PTI33 };

void gridpack::powerflow::PFApp::execute(int argc,
                                         char** argv,
                                         std::complex<double>& Vout,                 // output: bus-2 phasor
                                         const std::complex<double>& Sinj)           // input: aggregated S in pu
{
  gridpack::parallel::Communicator world;
  auto& S = state();

  if (!S.inited) {
    S.net.reset(new PFNetwork(world));

    S.cfg = gridpack::utility::Configuration::configuration();
    S.cfg->enableLogging(&std::cout);

    bool opened = false;
    if (argc >= 2 && argv[1] != nullptr) opened = S.cfg->open(argv[1], world);
    else                                 opened = S.cfg->open("118.xml", world);
    if (!opened) { if (world.rank()==0) std::cerr<<"PFApp: open config failed\n"; return; }

    S.cursor = S.cfg->getCursor("Configuration.Powerflow");
    S.baseMVA = S.cursor->get("baseMVA", 100.0);

    std::string filename;
    int filetype = PTI23;
    if (!S.cursor->get("networkConfiguration", &filename)) {
      if (S.cursor->get("networkConfiguration_v33", &filename)) filetype = PTI33;
      else { if (world.rank()==0) std::cerr<<"No networkConfiguration\n"; return; }
    }
    const double phaseShiftSign = S.cursor->get("phaseShiftSign", 1.0);

    if (filetype == PTI23) {
      if (world.rank()==0) std::cout<<"Using V23 parser for "<<filename<<"\n";
      gridpack::parser::PTI23_parser<PFNetwork> parser(S.net);
      parser.parse(filename.c_str());
      if (phaseShiftSign == -1.0) parser.changePhaseShiftSign();
    } else {
      if (world.rank()==0) std::cout<<"Using V33 parser for "<<filename<<"\n";
      gridpack::parser::PTI33_parser<PFNetwork> parser(S.net);
      parser.parse(filename.c_str());
      if (phaseShiftSign == -1.0) parser.changePhaseShiftSign();
    }

    // Locate original bus 2
    S.bus_idx = -1;
    for (int i=0; i<S.net->numBuses(); ++i) {
      if (S.net->getOriginalBusIndex(i) == 2) { S.bus_idx = i; break; }
    }
    if (S.bus_idx == -1) { if (world.rank()==0) std::cerr<<"Bus 2 not found\n"; return; }

    // One-time build
    S.net->partition();

    S.fact.reset(new gridpack::powerflow::PFFactory(S.net));
    S.fact->load();
    S.fact->setComponents();
    S.fact->setExchange();
    S.net->initBusUpdate();
    S.fact->setYBus();
    S.fact->setSBus();

    // Solver/maps
    S.fact->setMode(RHS);
    S.vMap.reset(new gridpack::mapper::BusVectorMap<PFNetwork>(S.net));
    S.PQ = S.vMap->mapToVector();

    S.fact->setMode(Jacobian);
    S.jMap.reset(new gridpack::mapper::FullMatrixMap<PFNetwork>(S.net));
    S.J = S.jMap->mapToMatrix();

    S.X.reset(S.PQ->clone());
    S.solver.reset(new gridpack::math::LinearSolver(*S.J));
    S.solver->configure(S.cursor);

    S.inited = true;
  }

  // Apply S (pu in MW/Mvar) and solve
  const double P_MW   = Sinj.real() * S.baseMVA;
  const double Q_Mvar = Sinj.imag() * S.baseMVA;
  S.net->getBusData(S.bus_idx)->setValue(LOAD_PL, P_MW, 0);
  S.net->getBusData(S.bus_idx)->setValue(LOAD_QL, Q_Mvar, 0);

  const double tolerance = S.cursor->get("tolerance", 1.0e-6);
  const int    maxIt     = S.cursor->get("maxIteration", 50);

  S.fact->setMode(RHS);
  S.vMap->mapToVector(S.PQ);

  S.fact->setMode(Jacobian);
  S.jMap->mapToMatrix(S.J);

  S.X->zero();
  S.solver->solve(*S.PQ, *S.X);
  auto tol = S.PQ->normInfinity();

  int it = 0;
  while (std::real(tol) > tolerance && it < maxIt) {
    S.fact->setMode(RHS);
    S.vMap->mapToBus(S.X);
    S.net->updateBuses();
    S.vMap->mapToVector(S.PQ);

    S.fact->setMode(Jacobian);
    S.jMap->mapToMatrix(S.J);

    S.X->zero();
    S.solver->solve(*S.PQ, *S.X);
    tol = S.PQ->normInfinity();
    ++it;
  }

  // Push solution and return bus-2 voltage
  S.fact->setMode(RHS);
  S.vMap->mapToBus(S.X);
  S.net->updateBuses();

  std::string phase = "A";
  if (argc >= 3 && argv[2] != nullptr) phase = argv[2];

  const double vmag     = S.net->getBus(S.bus_idx)->getVoltage();
  const double vang_deg = S.net->getBus(S.bus_idx)->getPhase(); // deg
  Vout = std::polar(vmag, vang_deg * PI / 180.0);

  if (world.rank() == 0) {
    const std::string filename_out = "bus_voltages_phase" + phase + ".csv";
    std::ofstream outFile(filename_out);
    outFile << "Original Bus Number,Voltage Magnitude (pu),Voltage Angle (deg)\n";
    for (int i = 0; i < S.net->numBuses(); ++i) {
      outFile << S.net->getOriginalBusIndex(i) << ","
              << S.net->getBus(i)->getVoltage() << ","
              << S.net->getBus(i)->getPhase()   << "\n";
    }
    outFile.close();
    std::cout << "Bus voltages written to " << filename_out << "\n";
  }
}

void gridpack::powerflow::PFApp::finalize() {
  auto& S = state();
  S.solver.reset();
  S.J.reset();
  S.X.reset();
  S.PQ.reset();
  S.jMap.reset();
  S.vMap.reset();
  S.fact.reset();
  S.net.reset();
  S.cfg = nullptr;
  S.bus_idx = -1;
  S.inited = false;
}
