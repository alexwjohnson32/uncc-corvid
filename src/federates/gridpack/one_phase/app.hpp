#pragma once

#include <vector>
#include <string>
#include <complex>

#include "common/utils/local_log_helper.hpp"

namespace one_phase
{

class PhaseApp
{
  public:
    PhaseApp();
    ~PhaseApp(); // This is required in order to use the forward declared inner class.

    void Initialize(const std::string &config_file, const std::vector<int> &bus_ids, const std::string &phase_name,
                    double rotation_degrees);
    std::complex<double> ComputeVoltage(int target_bus_id, const std::complex<double> &S,
                                        common::utils::LocalLogHelper &log);
    std::complex<double> GetInitialPhasedVoltages() const;

  private:
    std::vector<int> m_bus_ids;
    double m_rotation_degrees;
    std::string m_phase_name;
    std::string m_config_file;
};

} // namespace one_phase