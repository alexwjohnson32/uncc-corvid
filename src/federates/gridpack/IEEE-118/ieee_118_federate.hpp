#pragma once

#include "ieee_118_inputs.hpp"
#include "common/utils/local_log_helper.hpp"

#include <memory>

namespace ieee_118
{
struct GridpackEnvConfig
{
    int argc;
    char **argv;
};

class IEEE118Federate
{
  public:
    IEEE118Federate();
    ~IEEE118Federate();

    void Initialize(const ieee_118::IEEE118Input &input, ieee_118::GridpackEnvConfig config);
    void Run();
    void Close();

  private:
    ieee_118::IEEE118Input m_fed_input;
    common::utils::LocalLogHelper m_log;
    class FederateState; // Forward declaration, implemented in source
    std::unique_ptr<FederateState> m_state;
};
} // namespace ieee_118