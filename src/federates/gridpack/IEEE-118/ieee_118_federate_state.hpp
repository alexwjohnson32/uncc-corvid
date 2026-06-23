#pragma once

#include "ieee_118_app.hpp"
#include "ieee_118_inputs.hpp"
#include "common/utils/local_log_helper.hpp"
#include "common/helics/tools.hpp"

#include <helics/application_api/ValueFederate.hpp>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/Inputs.hpp>

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

namespace ieee_118
{
class FederateState
{
  public:
    std::shared_ptr<helics::ValueFederate> m_fed;
    common::helics::VoltagePublisher m_pub;
    std::unordered_map<std::string, common::helics::ThreePhaseSubscriptions> m_subs;
    std::unordered_map<std::string, common::helics::ThreePhaseValues> m_last_known_values;
    ieee_118::IEEE118App m_executor;
    std::vector<int> m_bus_ids;
    double m_period;

    FederateState();
    ~FederateState();

    void Initialize(const ieee_118::IEEE118Input &input, const std::shared_ptr<helics::ValueFederate> fed,
                    common::utils::LocalLogHelper &log);

    double RunSimulation(const ieee_118::IEEE118Input &input, common::utils::LocalLogHelper &log);

  private:
    double SimulateStep(const std::vector<ieee_118::GridlabDInputs> &gridlabd_infos, const double current_time,
                        common::utils::LocalLogHelper &log);
};
} // namespace ieee_118