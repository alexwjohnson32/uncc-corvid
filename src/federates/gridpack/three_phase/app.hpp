#pragma once

#include <vector>
#include <memory>
#include <string>
#include <complex>

#include "common/utils/local_log_helper.hpp"
#include "inputs.hpp"
#include "tools.hpp"

namespace three_phase
{

class PhaseApp
{
  public:
    PhaseApp();
    ~PhaseApp(); // This is required in order to use the forward declared inner class.

    bool Initialize(const std::string &config_file, const std::vector<int> &bus_ids, const std::string &phase_name,
                    double rotation_degrees, common::utils::LocalLogHelper &log, double ln_magnitude);
    std::complex<double> ComputeVoltageCurrent(double time_step, int target_bus_id, const std::complex<double> &Sa,
                                               common::utils::LocalLogHelper &log);
    double GetRotationAngle() const;

  private:
    class State; // forward declare, implement in source file
    std::unique_ptr<State> m_state;
    std::vector<int> m_bus_ids;
    double m_rotation_degrees;
    std::string m_phase_name;
    double m_ln_magnitude;
};

class ThreePhaseApp
{
  public:
    ThreePhaseApp();

    bool Initialize(const three_phase::PhaseInput &phase_a, const three_phase::PhaseInput &phase_b,
                    const three_phase::PhaseInput &phase_c, const std::vector<int> &bus_ids,
                    common::utils::LocalLogHelper &log, double ln_magnitude);

    common::helics::ThreePhaseValues ComputeVoltage(double time_step, const common::helics::ThreePhaseValues &power_s,
                                                    int bus_id, common::utils::LocalLogHelper &log);
    common::helics::ThreePhaseValues GetInitialPhasedVoltages() const;

  private:
    PhaseApp m_phase_a{};
    PhaseApp m_phase_b{};
    PhaseApp m_phase_c{};
};

} // namespace three_phase