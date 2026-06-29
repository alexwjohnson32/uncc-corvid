#pragma once

#include "ieee_118_inputs.hpp"
#include "ieee_118_federate_state.hpp"
#include "common/utils/local_log_helper.hpp"

#include <helics/application_api/ValueFederate.hpp>

namespace ieee_118
{
class IEEE118Federate
{
  public:
    IEEE118Federate();
    ~IEEE118Federate();

    void Initialize(const ieee_118::IEEE118Input &input, const std::shared_ptr<helics::ValueFederate> &fed);
    void Run();

  private:
    ieee_118::IEEE118Input m_fed_input;
    common::utils::LocalLogHelper m_log;
    ieee_118::FederateState m_state;
};
} // namespace ieee_118