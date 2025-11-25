#include "three_phase_pf_app.hpp"

#include <iostream>
#include <fstream>

#include "stopwatch.hpp"

bool gridpack::powerflow::ThreePhasePFApp::Initialize(const std::string &config_file, const std::vector<int> &bus_ids,
                                                      const std::complex<double> &r)
{
    m_config_file = config_file;
    m_bus_ids = bus_ids;
    m_r = r;

    bool success = m_state.InitializeConfig(m_config_file);
    if (!success)
    {
        std::cerr << "Could not initialize pf state with config file: " << m_config_file << std::endl;
        return success;
    }

    success = m_state.InitializeBusIndeces(m_bus_ids);
    if (!success)
    {
        std::cerr << "Could not initialize bus indeces with the following ids:\n";
        for (int bus_ids : m_bus_ids)
        {
            std::cerr << bus_ids << " ";
        }
        std::cerr << "/n";
        return success;
    }

    m_state.InitializeFactoryAndFields();

    return success;
}

gridpack::powerflow::ThreePhaseValues
gridpack::powerflow::ThreePhasePFApp::ComputeVoltage(const gridpack::powerflow::ThreePhaseValues &power_s, int bus_id)
{
    static std::ofstream output_console("three_phase_timimng.log");
    gridpack::powerflow::ThreePhaseValues phased_voltage;

    output_console << "####################################\n";
    output_console << "Bus Id: " << bus_id << "\n";
    output_console << "Power A: " << power_s.a << "\n";
    output_console << "Power B: " << power_s.b << "\n";
    output_console << "Power C: " << power_s.c << "\n";

    utils::Stopwatch watch;
    phased_voltage.a = m_app_A.ComputeVoltageCurrent(m_config_file, bus_id, "A", power_s.a, m_state);
    long long time_a = watch.ElapsedMilliseconds();
    output_console << "Time A: " << time_a << " ms\n";

    watch.Start();
    phased_voltage.b = m_app_B.ComputeVoltageCurrent(m_config_file, bus_id, "B", power_s.b, m_state) * m_r;
    long long time_b = watch.ElapsedMilliseconds();
    output_console << "Time B: " << time_b << " ms\n";

    watch.Start();
    phased_voltage.c = m_app_C.ComputeVoltageCurrent(m_config_file, bus_id, "C", power_s.c, m_state) * m_r * m_r;
    long long time_c = watch.ElapsedMilliseconds();
    output_console << "Time C: " << time_c << " ms\n";

    output_console << "####################################\n\n";

    return phased_voltage;
}