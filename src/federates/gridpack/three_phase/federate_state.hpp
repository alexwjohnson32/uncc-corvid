#pragma once

#include "common/helics/tools.hpp"
#include "common/utils/local_log_helper.hpp"
#include "app.hpp"

#include <helics/application_api/ValueFederate.hpp>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/Inputs.hpp>

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

namespace three_phase
{
class FederateState
{
  public:
    static std::unique_ptr<FederateState> Create(const three_phase::ThreePhaseInput &input,
                                                 const std::shared_ptr<helics::ValueFederate> &fed,
                                                 common::utils::LocalLogHelper &log);

    std::shared_ptr<helics::ValueFederate> m_fed;
    common::helics::VoltagePublisher m_pub;
    std::unordered_map<std::string, common::helics::ThreePhaseSubscriptions> m_subs;
    three_phase::ThreePhaseApp m_executor;
    std::vector<int> m_bus_ids;
    double m_period;
    double m_ln_magnitude;

    ~FederateState() {}

    double RunSimulation(const three_phase::ThreePhaseInput &input, common::utils::LocalLogHelper &log);

  private:
    FederateState();
    void Initialize(const three_phase::ThreePhaseInput &input, const std::shared_ptr<helics::ValueFederate> &fed,
                    common::utils::LocalLogHelper &log);
    double SimulateStep(const std::vector<three_phase::GridlabDInputs> &gridlabd_infos, const double current_time,
                        common::utils::LocalLogHelper &log);
};
} // namespace three_phase