#include <complex>

#include <helics/application_api/ValueFederate.hpp>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/Inputs.hpp>

#include "three_phase_pf_app.hpp"

namespace powerflow
{
namespace tools
{

struct ThreePhaseValues
{
    std::complex<double> a{ 0.0, 0.0 };
    std::complex<double> b{ 0.0, 0.0 };
    std::complex<double> c{ 0.0, 0.0 };
};

struct ThreePhaseSubscriptions
{
    helics::Input a{};
    helics::Input b{};
    helics::Input c{};
};

class VoltagePublisher
{
  private:
    helics::Publication m_a;
    helics::Publication m_b;
    helics::Publication m_c;
    const double m_ln_magnitude{};

  public:
    VoltagePublisher(helics::ValueFederate &fed, double ln_magnitude);

    void Publish(const gridpack::powerflow::ThreePhaseValues &v);
};

std::complex<double> LimitPower(const std::complex<double> &s, double max_v);
gridpack::powerflow::ThreePhaseValues LimitPower(ThreePhaseSubscriptions &sub, double max_v);

} // namespace tools
} // namespace powerflow