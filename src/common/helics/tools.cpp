#include "tools.hpp"

common::helics::VoltagePublisher::VoltagePublisher() : m_a{}, m_b{}, m_c{}, m_ln_magnitude{} {}

common::helics::VoltagePublisher::VoltagePublisher(::helics::ValueFederate &fed, double ln_magnitude)
    : m_ln_magnitude(ln_magnitude)
{
    m_a = fed.registerPublication("Va", "complex", "V");
    m_b = fed.registerPublication("Vb", "complex", "V");
    m_c = fed.registerPublication("Vc", "complex", "V");
}

void common::helics::VoltagePublisher::Publish(const common::helics::ThreePhaseValues &v)
{
    m_a.publish(v.a * m_ln_magnitude);
    m_b.publish(v.b * m_ln_magnitude);
    m_c.publish(v.c * m_ln_magnitude);
}

std::complex<double> common::helics::LimitPower(const std::complex<double> &s, double max_v)
{
    const double abs_s = std::abs(s);
    if (abs_s > max_v && abs_s > 0.0)
    {
        return s * (max_v / abs_s);
    }
    else
    {
        return s;
    }
}

common::helics::ThreePhaseValues common::helics::LimitPower(common::helics::ThreePhaseSubscriptions &sub, double max_v,
                                                            double divisor)
{
    common::helics::ThreePhaseValues limited_power;

    limited_power.a = LimitPower(sub.a.getValue<std::complex<double>>() / divisor, max_v);
    limited_power.b = LimitPower(sub.b.getValue<std::complex<double>>() / divisor, max_v);
    limited_power.c = LimitPower(sub.c.getValue<std::complex<double>>() / divisor, max_v);

    return limited_power;
}