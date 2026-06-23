#pragma once

#include "inputs.hpp"
#include "federate_state.hpp"
#include "common/utils/local_log_helper.hpp"

#include <memory>
#include <helics/application_api/ValueFederate.hpp>

namespace three_phase
{
class ThreePhaseFederate
{
  public:
    ThreePhaseFederate();
    ~ThreePhaseFederate();

    void Initialize(const three_phase::ThreePhaseInput &input, const std::shared_ptr<helics::ValueFederate> &fed);
    void Run();

  private:
    three_phase::ThreePhaseInput m_fed_input;
    common::utils::LocalLogHelper m_log;
    std::unique_ptr<three_phase::FederateState> m_state;
};
} // namespace three_phase