#pragma once

#include <vector>
#include <memory>
#include <string>
#include <complex>

#include "local_log_helper.hpp"
#include "tools.hpp"

namespace ieee_118
{

class IEEE118App
{
  public:
    IEEE118App();
    ~IEEE118App(); // This is required in order to use the forward declared inner class.

    bool Initialize(const std::string &config_file, const std::vector<int> &bus_ids, const std::complex<double> &r);
    powerflow::tools::ThreePhaseValues ComputeVoltage(const powerflow::tools::ThreePhaseValues &power_s, int bus_id);

  private:
    class State; // forward declare, implement in source file
    std::unique_ptr<State> m_state;
    std::string m_config_file;
    std::vector<int> m_bus_ids;
    std::complex<double> m_r;

    utils::LocalLogHelper m_log;

    std::complex<double> ComputeVoltageCurrent(const std::string &config_file, int target_bus_id,
                                               const std::string &phase_name, const std::complex<double> &Sa);
};
} // namespace ieee_118