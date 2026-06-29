#pragma once

#include "inputs.hpp"
#include "federate_state.hpp"
#include "common/utils/local_log_helper.hpp"

#include <memory>
#include <helics/application_api/ValueFederate.hpp>

namespace one_phase
{
class PhaseFederate
{
  public:
    PhaseFederate();
    ~PhaseFederate();

    void Initialize(const one_phase::PhaseInput &input, const std::shared_ptr<helics::ValueFederate> &fed);
    void Run();

  private:
    one_phase::PhaseInput m_fed_input;
    common::utils::LocalLogHelper m_log;
    std::unique_ptr<one_phase::FederateState> m_state;
};
} // namespace one_phase