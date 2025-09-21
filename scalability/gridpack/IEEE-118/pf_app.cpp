#include "gridpack/include/gridpack.hpp"
#include "pf_app.hpp"
#include "pf_factory.hpp"
#include <iostream>
#include <fstream>

#include "boost/smart_ptr/shared_ptr.hpp"

namespace
{
constexpr double PI = 3.14159265358979323846;
} // namespace

gridpack::powerflow::PFApp::PFApp(void) {}
gridpack::powerflow::PFApp::~PFApp(void) {}

std::complex<double> gridpack::powerflow::PFApp::ComputeVoltageCurrent(const std::string &config_file,
                                                                       int target_bus_id, const std::string &phase_name,
                                                                       const std::complex<double> &Sa,
                                                                       gridpack::powerflow::PFState &state) const
{
    // Apply S (pu in MW/Mvar) and solve
    const double P_MW = Sa.real() * state.base_MVA;
    const double Q_Mvar = Sa.imag() * state.base_MVA;

    const int bus_index = state.GetBusIndex(target_bus_id);

    state.network->getBusData(bus_index)->setValue(LOAD_PL, P_MW, 0);
    state.network->getBusData(bus_index)->setValue(LOAD_QL, Q_Mvar, 0);

    const double tolerance = state.cursor->get("tolerance", 1.0e-6);
    const int max_iteration = state.cursor->get("maxIteration", 50);

    state.pf_factory->setMode(RHS);
    state.v_map->mapToVector(*state.PQ);

    state.pf_factory->setMode(Jacobian);
    state.j_map->mapToMatrix(*state.J);

    state.X->zero();
    state.solver->solve(*state.PQ, *state.X);
    auto tol = state.PQ->normInfinity();

    int iterator = 0;
    while (std::real(tol) > tolerance && iterator < max_iteration)
    {
        state.pf_factory->setMode(RHS);
        state.v_map->mapToBus(*state.X);
        state.network->updateBuses();
        state.v_map->mapToVector(*state.PQ);

        state.pf_factory->setMode(Jacobian);
        state.j_map->mapToMatrix(*state.J);

        state.X->zero();
        state.solver->solve(*state.PQ, *state.X);
        tol = state.PQ->normInfinity();
        iterator++;
    }

    // Push solution and return bus id voltage
    state.pf_factory->setMode(RHS);
    state.v_map->mapToBus(*state.X);
    state.network->updateBuses();

    const double v_mag = state.network->getBus(bus_index)->getVoltage();
    const double v_ang_deg = state.network->getBus(bus_index)->getPhase(); // deg

    if (state.GetWorldRank() == 0)
    {
        const std::string filename_out = "bus_voltages_phase" + phase_name + ".csv";
        std::ofstream out_file(filename_out);

        out_file << "Original Bus Number,Voltage Magnitude (pu),Voltage Angle (deg)\n";
        for (int i = 0; i < state.network->numBuses(); i++)
        {
            out_file << state.network->getOriginalBusIndex(i) << "," << state.network->getBus(i)->getVoltage() << ","
                     << state.network->getBus(i)->getPhase() << "\n";
        }
        out_file.close();

        std::cout << "Bus voltages written to " << filename_out << "\n";
    }

    return std::polar(v_mag, v_ang_deg * PI / 180.0);
}