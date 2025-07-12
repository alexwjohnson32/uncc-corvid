#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <complex>

// GridPACK includes
#include "mpi.h"
#include <ga.h>
#include <macdecls.h>
#include "gridpack/include/gridpack.hpp"
#include "pf_app.hpp"

int main(int argc, char **argv) {


  std::ofstream outFile("gpk_118.csv");

  // Prepare GridPACK Environment
  gridpack::Environment env(argc, argv);
  gridpack::math::Initialize(&argc, &argv);
  gridpack::powerflow::PFApp app_A, app_B, app_C;

  auto Sa = std::complex<double>(0.0, 0.0);
  auto Sb = std::complex<double>(0.0, 0.0);
  auto Sc = std::complex<double>(0.0, 0.0);

  auto Va = std::complex<double>(1.0, 0.0);
  auto Vb = std::complex<double>(-0.5, -0.866025);
  auto Vc = std::complex<double>(-0.5, 0.866025);
  const auto r120 = std::complex<double>(-0.5, -0.866025);


  const char* xml_file = "118.xml";

  auto limitPower = [](std::complex<double> s, double max_mva) {
    return (std::abs(s) > max_mva) ? s * (max_mva / std::abs(s)) : s;
  };

  Sa = std::complex<double>(0.0419414,0.00742455);
  Sb = std::complex<double>(0.0378077,0.0051672);
  Sc = std::complex<double>(0.0418298,0.00271578);

  char* argsA[] = {argv[0], (char*)xml_file, (char*)"A"};
  char* argsB[] = {argv[0], (char*)xml_file, (char*)"B"};
  char* argsC[] = {argv[0], (char*)xml_file, (char*)"C"};

  app_A.execute(3, argsA, Va, Sa);
  app_B.execute(3, argsB, Vb, Sb);
  app_C.execute(3, argsC, Vc, Sc);

  Vb *= r120;
  Vc *= r120 * r120;

   std::cout << "Sa: " << Sa << ", Sb: " << Sb << ", Sc: " << Sc << "\n"
              << "Va: " << Va << ", Vb: " << Vb << ", Vc: " << Vc << "\n";

    outFile << "S Constant, Sa: " << Sa << " Sb: " << Sb << " Sc: " << Sc << "\n"
            << "V from GridPACK, Va: " << Va << " Vb: " << Vb << " Vc: " << Vc << "\n\n";


  outFile << "End of Simulation.";
  outFile.close();

  gridpack::math::Finalize();
  return 0;
}
