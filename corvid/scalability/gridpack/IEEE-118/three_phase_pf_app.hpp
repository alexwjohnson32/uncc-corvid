#pragma once

#include <string>
#include <vector>
#include <complex>

#include "pf_app.hpp"
#include "pf_state.hpp"

namespace gridpack
{
namespace powerflow
{

struct ThreePhaseValues
{
    std::complex<double> a;
    std::complex<double> b;
    std::complex<double> c;
};

class ThreePhasePFApp
{
  private:
    gridpack::powerflow::PFState m_state;
    gridpack::powerflow::PFApp m_app_A;
    gridpack::powerflow::PFApp m_app_B;
    gridpack::powerflow::PFApp m_app_C;

    std::string m_config_file;
    std::vector<int> m_bus_ids;
    std::complex<double> m_r;

  public:
    ThreePhasePFApp() : m_state(), m_app_A(), m_app_B(), m_app_C(), m_config_file(""), m_bus_ids(), m_r(0.0, 0.0) {}

    bool Initialize(const std::string &config_file, const std::vector<int> &bus_ids, const std::complex<double> &r);

    gridpack::powerflow::ThreePhaseValues ComputeVoltage(const gridpack::powerflow::ThreePhaseValues &power_s,
                                                         int bus_id);
};
} // namespace powerflow
} // namespace gridpack