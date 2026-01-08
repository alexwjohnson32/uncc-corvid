#include "tools.hpp"
#include <complex>

powerflow::tools::VoltagePublisher::VoltagePublisher(helics::ValueFederate &fed, double ln_magnitude)
    : m_ln_magnitude(ln_magnitude)
{
    m_a = fed.registerPublication("Va", "complex", "V");
    m_b = fed.registerPublication("Vb", "complex", "V");
    m_c = fed.registerPublication("Vc", "complex", "V");
}

void powerflow::tools::VoltagePublisher::Publish(const powerflow::tools::ThreePhaseValues &v)
{
    m_a.publish(v.a * m_ln_magnitude);
    m_b.publish(v.b * m_ln_magnitude);
    m_c.publish(v.c * m_ln_magnitude);
}

std::complex<double> powerflow::tools::LimitPower(const std::complex<double> &s, double max_v)
{
    const double abs_s = std::abs(s);
    if (abs_s > max_v)
    {
        return s * (max_v / abs_s);
    }
    else
    {
        return s;
    }
}

powerflow::tools::ThreePhaseValues powerflow::tools::LimitPower(powerflow::tools::ThreePhaseSubscriptions &sub,
                                                                   double max_v)
{
    powerflow::tools::ThreePhaseValues limited_power;

    limited_power.a = LimitPower(sub.a.getValue<std::complex<double>>() / 1e8, max_v);
    limited_power.b = LimitPower(sub.b.getValue<std::complex<double>>() / 1e8, max_v);
    limited_power.c = LimitPower(sub.c.getValue<std::complex<double>>() / 1e8, max_v);

    return limited_power;
}