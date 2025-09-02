#include <iostream>
#include <fstream>
#include <iomanip>
#include <complex>
#include <string>

// GridPACK
#include "mpi.h"
#include <ga.h>
#include <macdecls.h>
#include "gridpack/include/gridpack.hpp"
#include "pf_app.hpp"

int main(int argc, char** argv) {
  // Init GridPACK/PETSc
  gridpack::Environment env(argc, argv);
  gridpack::math::Initialize(&argc, &argv);

  const char* xml_file = "118.xml";
  const std::complex<double> r120(-0.5, -0.8660254037844386);

  // Aggregated S from co-sim (pu on baseMVA)
  std::complex<double> Sa_pu(0.420172,0.0800222);
  std::complex<double> Sb_pu(0.386903,0.0695264);
  std::complex<double> Sc_pu(0.412627,0.0459622);

  // PF apps (one per phase)
  gridpack::powerflow::PFApp appA, appB, appC;

  // PFApp argv: program, XML, phase tag
  char* argsA[] = {argv[0], const_cast<char*>(xml_file), const_cast<char*>("A")};
  char* argsB[] = {argv[0], const_cast<char*>(xml_file), const_cast<char*>("B")};
  char* argsC[] = {argv[0], const_cast<char*>(xml_file), const_cast<char*>("C")};

  // Outputs
  std::complex<double> Va(0,0), Vb(0,0), Vc(0,0);

  // Solve with per-phase injections
  appA.execute(3, argsA, Va, Sa_pu);
  appB.execute(3, argsB, Vb, Sb_pu);
  appC.execute(3, argsC, Vc, Sc_pu);

  // Rotate B and C into ABC frame
  Vb *= r120;
  Vc *= (r120 * r120);

  std::cout << "Injected S (pu): Sa=" << Sa_pu << " Sb=" << Sb_pu << " Sc=" << Sc_pu << "\n"
            << "Bus 2 Voltages (pu): Va=" << Va << " Vb=" << Vb << " Vc=" << Vc << "\n";

  // Output Ss and Vs
  std::ofstream outFile("gpk_118_10_conn.csv");
  outFile << std::fixed << std::setprecision(6);
  outFile << "S Constant, Sa: " << Sa_pu
          << " Sb: " << Sb_pu
          << " Sc: " << Sc_pu << "\n"
          << "V from GridPACK, Va: " << Va
          << " Vb: " << Vb
          << " Vc: " << Vc << "\n\n";
  outFile.close();

  // Tear down PFApp cached objects BEFORE PETSc finalize
  appA.finalize();
  appB.finalize();
  appC.finalize();
  gridpack::math::Finalize();
  return 0;
}
