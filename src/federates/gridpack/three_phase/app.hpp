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

    bool Initialize(const std::string &config_file, const std::vector<int> &bus_ids,
                    common::utils::LocalLogHelper &log);
    std::complex<double> ComputeVoltageCurrent(int target_bus_id, const std::complex<double> &Sa,
                                               const std::string &phase_name,
                                               const std::complex<double> rotation_radians);

  private:
    class State; // forward declare, implement in source file
    std::unique_ptr<State> m_state;
};

class Rotation
{
  public:
    Rotation(double a, double b, double c);
    std::complex<double> GetARad() const;
    std::complex<double> GetBRad() const;
    std::complex<double> GetCRad() const;

  private:
    double m_a{};
    double m_b{};
    double m_c{};

    std::complex<double> GetRotation(double rot) const;
};

class ThreePhaseApp
{
  public:
    ThreePhaseApp();

    bool Initialize(const std::string &xml_file, double a_rotation_degrees, double b_rotation_degrees,
                    double c_rotation_degrees, const std::vector<int> &bus_ids, common::utils::LocalLogHelper &log);

    common::helics::ThreePhaseValues ComputeVoltage(const common::helics::ThreePhaseValues &power_s, int bus_id,
                                                    common::utils::LocalLogHelper &log);
    common::helics::ThreePhaseValues GetInitialPhasedVoltages() const;

  private:
    PhaseApp m_phase{};
    Rotation m_rotation{ 0.0, 0.0, 0.0 };
};

} // namespace three_phase