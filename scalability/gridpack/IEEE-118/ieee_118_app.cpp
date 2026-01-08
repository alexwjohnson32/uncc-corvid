#include "ieee_118_app.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

#include "stopwatch.hpp"

bool ieee_118::IEEE118App::Initialize(const std::string &config_file, const std::vector<int> &bus_ids,
                                      const std::complex<double> &r)
{
    m_config_file = config_file;
    m_bus_ids = bus_ids;
    m_r = r;

    bool success = m_state.InitializeConfig(m_config_file);
    if (!success)
    {
        m_log << "Could not initialize pf state with config file: " << m_config_file << std::endl;
        return success;
    }

    success = m_state.InitializeBusIndeces(m_bus_ids);
    if (!success)
    {
        std::stringstream out;
        out << "Could not initialize bus indeces with the following ids:\n";
        for (int bus_ids : m_bus_ids)
        {
            out << bus_ids << " ";
        }
        out << "/n";
        m_log << out.str();
        return success;
    }

    m_state.InitializeFactoryAndFields();

    return success;
}

powerflow::tools::ThreePhaseValues
ieee_118::IEEE118App::ComputeVoltage(const powerflow::tools::ThreePhaseValues &power_s, int bus_id)
{
    powerflow::tools::ThreePhaseValues phased_voltage;

    std::stringstream out;
    out << "####################################\n";
    out << "Bus Id: " << bus_id << "\n";
    out << "Power A: " << power_s.a << "\n";
    out << "Power B: " << power_s.b << "\n";
    out << "Power C: " << power_s.c << "\n";

    utils::Stopwatch watch;
    phased_voltage.a = m_state.ComputeVoltageCurrent(m_config_file, bus_id, "A", power_s.a);
    long long time_a = watch.ElapsedMilliseconds();
    out << "Time A: " << time_a << " ms\n";

    watch.Start();
    phased_voltage.b = m_state.ComputeVoltageCurrent(m_config_file, bus_id, "B", power_s.b) * m_r;
    long long time_b = watch.ElapsedMilliseconds();
    out << "Time B: " << time_b << " ms\n";

    watch.Start();
    phased_voltage.c = m_state.ComputeVoltageCurrent(m_config_file, bus_id, "C", power_s.c) * m_r * m_r;
    long long time_c = watch.ElapsedMilliseconds();
    out << "Time C: " << time_c << " ms\n";

    out << "####################################\n\n";

    m_log << out.str();

    return phased_voltage;
}