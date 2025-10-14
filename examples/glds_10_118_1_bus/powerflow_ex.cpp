#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <complex>
#include <string>
#include <helics/application_api/ValueFederate.hpp>

// GridPACK includes
#include "mpi.h"
#include <ga.h>
#include <macdecls.h>
#include "gridpack/include/gridpack.hpp"
#include "pf_app.hpp"

// Aggregating the number of GridLAB-D models in the simulation
static inline std::complex<double> limitPower(const std::complex<double>& s, double max_mva) {
  const double mag = std::abs(s);
  return (mag > max_mva && mag > 0.0) ? s * (max_mva / mag) : s;
}

int main(int argc, char **argv) {
  // HELICS Federate
  helics::FederateInfo fi;
  fi.coreType = helics::CoreType::ZMQ;
  fi.coreInitString = "--federates=1";
  fi.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_DEBUG);
  const double period = 1.0;
  fi.setProperty(HELICS_PROPERTY_TIME_PERIOD, period);
  fi.setFlagOption(HELICS_FLAG_UNINTERRUPTIBLE, false);
  fi.setFlagOption(HELICS_FLAG_TERMINATE_ON_ERROR, true);
  fi.setFlagOption(HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE, true);

  helics::ValueFederate gpk_118("gridpack", fi);
  std::cout << "HELICS GridPACK Federate created successfully.\n";

  // Publications
  auto Va_pub = gpk_118.registerPublication("Va", "complex", "V");
  auto Vb_pub = gpk_118.registerPublication("Vb", "complex", "V");
  auto Vc_pub = gpk_118.registerPublication("Vc", "complex", "V");

  // --- Subscriptions (aggregate N GLMs)
  const int N = 10; // number of GLMs to aggregate
  std::vector<helics::Input> Sa_in, Sb_in, Sc_in;
  Sa_in.reserve(N); Sb_in.reserve(N); Sc_in.reserve(N);
  for (int i = 1; i <= N; ++i) {
    Sa_in.emplace_back(gpk_118.registerSubscription("gld_hlc_conn_" + std::to_string(i) + "/Sa", "VA"));
    Sb_in.emplace_back(gpk_118.registerSubscription("gld_hlc_conn_" + std::to_string(i) + "/Sb", "VA"));
    Sc_in.emplace_back(gpk_118.registerSubscription("gld_hlc_conn_" + std::to_string(i) + "/Sc", "VA"));
  }

  std::ofstream outFile("gpk_118_10_bus_conn.csv");

  // Last-known values to avoid stalls
  std::vector<std::complex<double>> Sa_last(N, {0.0,0.0}), Sb_last(N, {0.0,0.0}), Sc_last(N, {0.0,0.0});

  // GridPACK
  gridpack::Environment env(argc, argv);
  gridpack::math::Initialize(&argc, &argv);
  gridpack::powerflow::PFApp app_A, app_B, app_C;

  // Exec mode
  gpk_118.enterExecutingModeAsync();
  gpk_118.enterExecutingModeComplete();
  std::cout << "GridPACK Federate has entered execution mode.\n";

  const double total_interval = 300; //86400;	//3600;	//120.0;
  double grantedtime = 0.0;

  const auto r120 = std::complex<double>(-0.5, -0.866025);
  const char* xml_file = "118.xml";

  std::complex<double> Va_val(0,0), Vb_val(0,0), Vc_val(0,0);

  while (grantedtime + period <= total_interval) {
    grantedtime = gpk_118.requestTime(grantedtime + period);

    // Aggregate S from N GLMs (GLD sends VA -> convert to pu by /1e8; clamp to 1.0 pu)
    std::complex<double> Sa_tot(0,0), Sb_tot(0,0), Sc_tot(0,0);
    for (int i = 0; i < N; ++i) {
      if (Sa_in[i].isUpdated() || Sa_in[i].isValid()) Sa_last[i] = Sa_in[i].getValue<std::complex<double>>();
      if (Sb_in[i].isUpdated() || Sb_in[i].isValid()) Sb_last[i] = Sb_in[i].getValue<std::complex<double>>();
      if (Sc_in[i].isUpdated() || Sc_in[i].isValid()) Sc_last[i] = Sc_in[i].getValue<std::complex<double>>();


      Sa_tot += limitPower(Sa_last[i] / 1e8, 1.0);
      Sb_tot += limitPower(Sb_last[i] / 1e8, 1.0);
      Sc_tot += limitPower(Sc_last[i] / 1e8, 1.0);
    }
    auto Sa = Sa_tot, Sb = Sb_tot, Sc = Sc_tot;

    // PFApp args
    char* argsA[] = {argv[0], (char*)xml_file, (char*)"A"};
    char* argsB[] = {argv[0], (char*)xml_file, (char*)"B"};
    char* argsC[] = {argv[0], (char*)xml_file, (char*)"C"};

    // Single-bus voltage output signature: execute(argc, argv, Vout, Sinj)
    app_A.execute(3, argsA, Va_val, Sa);
    app_B.execute(3, argsB, Vb_val, Sb);
    app_C.execute(3, argsC, Vc_val, Sc);

    // Rotate to ABC frame
    Vb_val *= r120;
    Vc_val *= (r120 * r120);

    std::cout << "[Time " << grantedtime << "]\n"
              << "Sa: " << Sa << ", Sb: " << Sb << ", Sc: " << Sc << "\n"
              << "Bus 2: Va: " << Va_val << ", Vb: " << Vb_val << ", Vc: " << Vc_val << "\n";

    outFile << "Time (s): " << grantedtime << "\n"
            << "S received from Gridlab-D, Sa: " << Sa << " Sb: " << Sb << " Sc: " << Sc << "\n"
            << "Updated V by GridPACK:\n"
            << "Bus 2: Va: " << Va_val << ", Vb: " << Vb_val << ", Vc: " << Vc_val << "\n\n";

    // Publish phase voltages (scale to volts)
    Va_pub.publish(Va_val * 79600.0);
    Vb_pub.publish(Vb_val * 79600.0);
    Vc_pub.publish(Vc_val * 79600.0);
  }

  outFile << "End of Cosimulation.";
  outFile.close();

  app_A.finalize();
  app_B.finalize();
  app_C.finalize();

  gridpack::math::Finalize();
  gpk_118.finalize();
  std::cout << "Federate finalized.\nGranted time: " << grantedtime << std::endl;
  return 0;
}
