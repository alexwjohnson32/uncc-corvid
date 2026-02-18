#pragma once

#include "common/helics/helics_input.hpp"
#include "common/utils/local_log_helper.hpp"

namespace dummy
{

class DummyFederate
{
  public:
    DummyFederate();
    ~DummyFederate();

    void Initialize(const common::helics::HelicsInput &input);
    void Run();
    void Close();

  private:
    common::helics::HelicsInput m_fed_input;
    common::utils::LocalLogHelper m_log;
};

} // namespace data