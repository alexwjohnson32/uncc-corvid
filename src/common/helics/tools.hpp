#pragma once

#include <complex>

#include <helics/application_api/ValueFederate.hpp>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/Inputs.hpp>

namespace common
{
namespace helics
{

struct ThreePhaseValues
{
    std::complex<double> a{ 0.0, 0.0 };
    std::complex<double> b{ 0.0, 0.0 };
    std::complex<double> c{ 0.0, 0.0 };
};

struct ThreePhaseSubscriptions
{
    ::helics::Input a{};
    ::helics::Input b{};
    ::helics::Input c{};
};

class VoltagePublisher
{
  private:
    ::helics::Publication m_a;
    ::helics::Publication m_b;
    ::helics::Publication m_c;
    double m_ln_magnitude{};

  public:
    VoltagePublisher();
    VoltagePublisher(::helics::ValueFederate &fed, double ln_magnitude);

    void Publish(const ThreePhaseValues &v);
};

std::complex<double> LimitPower(const std::complex<double> &s, double max_v);
ThreePhaseValues LimitPower(ThreePhaseSubscriptions &sub, double max_v, double divisor = 1e8);

} // namespace tools
} // namespace powerflow