#pragma once

#include <string>
#include <vector>
#include <complex>

#include "state.hpp"
#include "local_log_helper.hpp"

#include "tools.hpp"

namespace ieee_118
{

class IEEE118App
{
  private:
    powerflow::State m_state;
    std::string m_config_file;
    std::vector<int> m_bus_ids;
    std::complex<double> m_r;

    utils::LocalLogHelper m_log;

  public:
    IEEE118App() : m_state(), m_config_file(""), m_bus_ids(), m_r(0.0, 0.0), m_log("three_phase_timimng.log") {}

    bool Initialize(const std::string &config_file, const std::vector<int> &bus_ids, const std::complex<double> &r);

    powerflow::tools::ThreePhaseValues ComputeVoltage(const powerflow::tools::ThreePhaseValues &power_s, int bus_id);
};
} // namespace ieee_118