#include "three_phase_pf_app.hpp"

#include <iostream>

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
    gridpack::powerflow::ThreePhaseValues phased_voltage;

    phased_voltage.a = m_app_A.ComputeVoltageCurrent(m_config_file, bus_id, "A", power_s.a, m_state);
    phased_voltage.b = m_app_B.ComputeVoltageCurrent(m_config_file, bus_id, "B", power_s.b, m_state) * m_r;
    phased_voltage.c = m_app_C.ComputeVoltageCurrent(m_config_file, bus_id, "C", power_s.c, m_state) * m_r * m_r;

    return phased_voltage;
}