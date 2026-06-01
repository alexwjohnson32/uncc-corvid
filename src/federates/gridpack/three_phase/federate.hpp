#pragma once

#include "inputs.hpp"
#include "federate_state.hpp"
#include "common/utils/local_log_helper.hpp"

#include <memory>

namespace three_phase
{
struct GridpackEnvConfig
{
    int argc;
    char **argv;
};

class ThreePhaseFederate
{
  public:
    ThreePhaseFederate();
    ~ThreePhaseFederate();

    void Initialize(const three_phase::ThreePhaseInput &input, three_phase::GridpackEnvConfig config);
    void Run();
    void Close();

  private:
    three_phase::ThreePhaseInput m_fed_input;
    common::utils::LocalLogHelper m_log;
    std::unique_ptr<three_phase::FederateState> m_state;
};
} // namespace three_phase