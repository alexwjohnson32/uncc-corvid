#pragma once

#include "query_federate_input.hpp"
#include "local_log_helper.hpp"
#include "websocket_client.hpp"

namespace queryable
{

class QueryFederate
{
  public:
    QueryFederate();
    ~QueryFederate();

    void Initialize(const queryable::QueryFederateInput &input);
    void Run();
    void Close();

  private:
    queryable::QueryFederateInput m_query_fed_input;
    common::utils::LocalLogHelper m_log;
    std::shared_ptr<common::utils::WebSocketClient> m_client;
};

} // namespace queryable