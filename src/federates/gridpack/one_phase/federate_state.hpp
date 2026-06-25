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

#include "inputs.hpp"

namespace one_phase
{
class FederateState
{
  public:
    static std::unique_ptr<FederateState> Create(const one_phase::PhaseInput &input,
                                                 const std::shared_ptr<helics::ValueFederate> &fed,
                                                 common::utils::LocalLogHelper &log);

    std::shared_ptr<helics::ValueFederate> m_fed;
    ::helics::Publication m_pub;
    std::unordered_map<std::string, ::helics::Input> m_subs;
    one_phase::PhaseApp m_app;
    std::vector<int> m_bus_ids;
    double m_period;
    double m_ln_magnitude;

    ~FederateState() {}

    double RunSimulation(const one_phase::PhaseInput &input, common::utils::LocalLogHelper &log);

  private:
    FederateState();
    void Initialize(const one_phase::PhaseInput &input, const std::shared_ptr<helics::ValueFederate> &fed,
                    common::utils::LocalLogHelper &log);
    double SimulateStep(const std::vector<one_phase::GridlabDInputs> &gridlabd_infos, const double current_time,
                        common::utils::LocalLogHelper &log);
};
} // namespace one_phase