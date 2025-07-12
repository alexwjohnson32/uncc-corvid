#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <complex>
#include <helics/application_api/ValueFederate.hpp>

// GridPACK includes
#include "mpi.h"
#include <ga.h>
#include <macdecls.h>
#include "gridpack/include/gridpack.hpp"
#include "pf_app.hpp"

int main(int argc, char **argv) {
  // Create a FederateInfo object
  helics::FederateInfo fi;
  fi.coreType = helics::CoreType::ZMQ;
  fi.coreInitString = "--federates=1";
  fi.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_DEBUG);
  double period = 1.0;
  fi.setProperty(HELICS_PROPERTY_TIME_PERIOD, period);
  fi.setFlagOption(HELICS_FLAG_UNINTERRUPTIBLE, false);
  fi.setFlagOption(HELICS_FLAG_TERMINATE_ON_ERROR, true);
  fi.setFlagOption(HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE, true);

  helics::ValueFederate gpk_118("gridpack", fi);
  std::cout << "HELICS GridPACK Federate created successfully." << std::endl;

  // Publications
  auto Va_id = gpk_118.registerPublication("Va", "complex", "V");
  auto Vb_id = gpk_118.registerPublication("Vb", "complex", "V");
  auto Vc_id = gpk_118.registerPublication("Vc", "complex", "V");

  // Subscriptions
  auto Sa_id = gpk_118.registerSubscription("gld_hlc_conn/Sa", "VA");
  auto Sb_id = gpk_118.registerSubscription("gld_hlc_conn/Sb", "VA");
  auto Sc_id = gpk_118.registerSubscription("gld_hlc_conn/Sc", "VA");

  std::ofstream outFile("gpk_118.csv");

  // Prepare GridPACK Environment
  gridpack::Environment env(argc, argv);
  gridpack::math::Initialize(&argc, &argv);
  gridpack::powerflow::PFApp app_A, app_B, app_C;

  // Enter execution mode
  gpk_118.enterExecutingMode();
  std::cout << "GridPACK Federate has entered execution mode." << std::endl;

  double total_interval = 60.0;
  double grantedtime = 0.0;

  auto Sa = std::complex<double>(0.0, 0.0);
  auto Sb = std::complex<double>(0.0, 0.0);
  auto Sc = std::complex<double>(0.0, 0.0);

  auto Va = std::complex<double>(1.0, 0.0);
  auto Vb = std::complex<double>(-0.5, -0.866025);
  auto Vc = std::complex<double>(-0.5, 0.866025);
  const auto r120 = std::complex<double>(-0.5, -0.866025);

  // Initial voltage publish (LN magnitude = 79600V)
  Va_id.publish(Va * 79600.0);
  Vb_id.publish(Vb * 79600.0);
  Vc_id.publish(Vc * 79600.0);

  const char* xml_file = "118.xml";

  auto limitPower = [](std::complex<double> s, double max_mva) {
    return (std::abs(s) > max_mva) ? s * (max_mva / std::abs(s)) : s;
  };

  while (grantedtime + period <= total_interval) {
    grantedtime = gpk_118.requestTime(grantedtime + period);

    Sa = limitPower(Sa_id.getValue<std::complex<double>>() / 1e8, 1.0);
    Sb = limitPower(Sb_id.getValue<std::complex<double>>() / 1e8, 1.0);
    Sc = limitPower(Sc_id.getValue<std::complex<double>>() / 1e8, 1.0);

    char* argsA[] = {argv[0], (char*)xml_file, (char*)"A"};
    char* argsB[] = {argv[0], (char*)xml_file, (char*)"B"};
    char* argsC[] = {argv[0], (char*)xml_file, (char*)"C"};

    app_A.execute(3, argsA, Va, Sa);
    app_B.execute(3, argsB, Vb, Sb);
    app_C.execute(3, argsC, Vc, Sc);

    Vb *= r120;
    Vc *= r120 * r120;

    std::cout << "[Time " << grantedtime << "]\n"
              << "Sa: " << Sa << ", Sb: " << Sb << ", Sc: " << Sc << "\n"
              << "Va: " << Va << ", Vb: " << Vb << ", Vc: " << Vc << "\n";

    outFile << "Time (s): " << grantedtime << "\n"
            << "S received from Gridlab-D, Sa: " << Sa << " Sb: " << Sb << " Sc: " << Sc << "\n"
            << "Updated V by GridPACK, Va: " << Va << " Vb: " << Vb << " Vc: " << Vc << "\n\n";

    Va_id.publish(Va * 79600.0);
    Vb_id.publish(Vb * 79600.0);
    Vc_id.publish(Vc * 79600.0);
  }

  outFile << "End of Cosimulation.";
  outFile.close();

  gridpack::math::Finalize();
  gpk_118.finalize();
  std::cout << "Federate finalized.\nGranted time: " << grantedtime << std::endl;
  return 0;
}
