#pragma once

#include <string>

#include "common/helics/helics_input.hpp"

#include <boost/json.hpp>

namespace queryable
{

struct ClientDetails
{
    std::string host{};
    std::string port{};
    std::string target{};
};

struct QueryFederateInput : public common::helics::HelicsInput
{
    ClientDetails client_details{};
};

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const queryable::ClientDetails &data);
queryable::ClientDetails tag_invoke(boost::json::value_to_tag<queryable::ClientDetails>,
                                    const boost::json::value &json_value);

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const queryable::QueryFederateInput &data);
queryable::QueryFederateInput tag_invoke(boost::json::value_to_tag<queryable::QueryFederateInput>,
                                         const boost::json::value &json_value);

} // namespace queryable