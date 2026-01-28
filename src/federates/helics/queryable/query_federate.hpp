#pragma once

#include "query_federate_input.hpp"
#include "local_log_helper.hpp"
#include "websocket_client.hpp"

#include <helics/application_api/MessageFederate.hpp>

namespace data
{

class QueryFederate
{
  public:
    QueryFederate();
    ~QueryFederate();

    void Initialize(const data::QueryFederateInput &input);
    void Run();
    void Close();

  private:
    data::QueryFederateInput m_query_fed_input;
    utils::LocalLogHelper m_log;
    std::shared_ptr<utils::WebSocketClient> m_client;
};

} // namespace data