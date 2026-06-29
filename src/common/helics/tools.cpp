#include "tools.hpp"

common::helics::ThreePhaseValues common::helics::ThreePhaseValues::Multiply(double scalar) const
{
    return { { a * scalar }, { b * scalar }, { c * scalar } };
}

common::helics::ThreePhaseValues common::helics::ThreePhaseSubscriptions::GetValues()
{
    return { a.getValue<std::complex<double>>(), b.getValue<std::complex<double>>(),
             c.getValue<std::complex<double>>() };
}

common::helics::ThreePhaseVoltagePublisher::ThreePhaseVoltagePublisher() : m_a{}, m_b{}, m_c{} {}

common::helics::ThreePhaseVoltagePublisher::ThreePhaseVoltagePublisher(::helics::ValueFederate &fed)
{
    m_a = fed.registerPublication("Va", "complex", "V");
    m_b = fed.registerPublication("Vb", "complex", "V");
    m_c = fed.registerPublication("Vc", "complex", "V");
}

common::helics::ThreePhaseVoltagePublisher::ThreePhaseVoltagePublisher(const std::shared_ptr<::helics::ValueFederate> fed)
{
    m_a = fed->registerPublication("Va", "complex", "V");
    m_b = fed->registerPublication("Vb", "complex", "V");
    m_c = fed->registerPublication("Vc", "complex", "V");
}

void common::helics::ThreePhaseVoltagePublisher::Publish(const common::helics::ThreePhaseValues &v)
{
    m_a.publish(v.a);
    m_b.publish(v.b);
    m_c.publish(v.c);
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

common::helics::ThreePhaseValues common::helics::LimitPower(const ThreePhaseValues &sub, double max_v, double divisor)
{
    return { LimitPower(sub.a / divisor, max_v), LimitPower(sub.b / divisor, max_v),
             LimitPower(sub.c / divisor, max_v) };
}