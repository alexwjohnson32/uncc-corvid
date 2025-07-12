#include "gridpack/include/gridpack.hpp"
#include "pf_app.hpp"
#include "pf_factory.hpp"
#include <iostream>
#include <fstream>
#include <string>

gridpack::powerflow::PFApp::PFApp(void) {}
gridpack::powerflow::PFApp::~PFApp(void) {}

enum Parser { PTI23, PTI33 };

void gridpack::powerflow::PFApp::execute(int argc,
                                         char** argv,
                                         std::complex<double>& Vc,
                                         std::complex<double>& Sa)
{
  gridpack::parallel::Communicator world;
  boost::shared_ptr<PFNetwork> network(new PFNetwork(world));

  gridpack::utility::Configuration *config = gridpack::utility::Configuration::configuration();
  config->enableLogging(&std::cout);
  bool opened;
  if (argc >= 2 && argv[1] != NULL) {
    opened = config->open(argv[1], world);
  } else {
    opened = config->open("118.xml", world);
  }
  if (!opened) return;

  gridpack::utility::Configuration::CursorPtr cursor;
  cursor = config->getCursor("Configuration.Powerflow");
  std::string filename;
  int filetype = PTI23;
  if (!cursor->get("networkConfiguration", &filename)) {
    if (cursor->get("networkConfiguration_v33", &filename)) {
      filetype = PTI33;
    } else {
      printf("No network configuration file specified\n");
      return;
    }
  }

  double tolerance = cursor->get("tolerance", 1.0e-6);
  int max_iteration = cursor->get("maxIteration", 50);
  double phaseShiftSign = cursor->get("phaseShiftSign", 1.0);

  if (world.rank() == 0) printf("Network filename: (%s)\n", filename.c_str());
  if (filetype == PTI23) {
    if (world.rank() == 0) printf("Using V23 parser\n");
    gridpack::parser::PTI23_parser<PFNetwork> parser(network);
    parser.parse(filename.c_str());
    if (phaseShiftSign == -1.0) parser.changePhaseShiftSign();
  } else if (filetype == PTI33) {
    if (world.rank() == 0) printf("Using V33 parser\n");
    gridpack::parser::PTI33_parser<PFNetwork> parser(network);
    parser.parse(filename.c_str());
    if (phaseShiftSign == -1.0) parser.changePhaseShiftSign();
  }

  // Inject complex power Sa into the target bus
  int target_orig_bus = 2;
  int matched_index = -1;
  for (int i = 0; i < network->numBuses(); ++i) {
    if (network->getOriginalBusIndex(i) == target_orig_bus) {
      matched_index = i;
      break;
    }
  }

  if (matched_index == -1) {
    std::cerr << "Original bus number " << target_orig_bus << " not found.\n";
    return;
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
  gridpack::mapper::BusVectorMap<PFNetwork> vMap(network);
  boost::shared_ptr<gridpack::math::Vector> PQ = vMap.mapToVector();

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

  while (real(tol) > tolerance && iter < max_iteration) {
    factory.setMode(RHS);
    vMap.mapToBus(X);
    network->updateBuses();
    vMap.mapToVector(PQ);
    factory.setMode(Jacobian);
    jMap.mapToMatrix(J);
    X->zero();
    solver.solve(*PQ, *X);
    tol = PQ->normInfinity();
    iter++;
  }

  factory.setMode(RHS);
  vMap.mapToBus(X);
  network->updateBuses();

  // Output file name logic
  std::string phase = "A";  // default
  if (argc >= 3 && argv[2] != NULL) {
    phase = argv[2];
  }

  std::string filename_out = "bus_voltages_phase" + phase + ".csv";

  // Set return voltage
  Vc = std::polar(network->getBus(matched_index)->getVoltage(),
                  network->getBus(matched_index)->getPhase());

  // Write voltages for all buses
  if (world.rank() == 0) {
    std::ofstream outFile(filename_out);
    outFile << "Original Bus Number,Voltage Magnitude (pu),Voltage Angle (deg)\n";
    for (int i = 0; i < network->numBuses(); ++i) {
      int busnum = network->getOriginalBusIndex(i);
      double vmag = network->getBus(i)->getVoltage();
      double vang = network->getBus(i)->getPhase();
      outFile << busnum << "," << vmag << "," << vang << "\n";
    }
    outFile.close();
    std::cout << "Bus voltages written to " << filename_out << "\n";
  }
}

