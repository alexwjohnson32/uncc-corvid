#pragma once

#include <complex>

#include <helics/application_api/ValueFederate.hpp>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/Inputs.hpp>
#include <memory>

namespace common
{
namespace helics
{

struct ThreePhaseValues
{
    std::complex<double> a{ 0.0, 0.0 };
    std::complex<double> b{ 0.0, 0.0 };
    std::complex<double> c{ 0.0, 0.0 };

    ThreePhaseValues Multiply(double scalar) const;
};

struct ThreePhaseSubscriptions
{
    ::helics::Input a{};
    ::helics::Input b{};
    ::helics::Input c{};

    ThreePhaseValues GetValues();
};

class ThreePhaseVoltagePublisher
{
  private:
    ::helics::Publication m_a;
    ::helics::Publication m_b;
    ::helics::Publication m_c;

  public:
    ThreePhaseVoltagePublisher();
    ThreePhaseVoltagePublisher(::helics::ValueFederate &fed);
    ThreePhaseVoltagePublisher(const std::shared_ptr<::helics::ValueFederate> fed);

    void Publish(const ThreePhaseValues &v);
};

std::complex<double> LimitPower(const std::complex<double> &s, double max_v);
ThreePhaseValues LimitPower(const ThreePhaseValues &sub, double max_v, double divisor = 1e8);

} // namespace helics
} // namespace common