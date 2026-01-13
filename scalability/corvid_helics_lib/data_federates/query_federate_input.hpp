#pragma once

#include <string>

#include <boost/json.hpp>

namespace data
{

struct ClientDetails
{
    std::string host{};
    std::string port{};
    std::string target{};
};

struct QueryFederateInput
{
    std::string federate_name{};
    std::string fed_info_json{};
    ClientDetails client_details{};
    double total_time{};
    std::string local_log_file{};
};

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const data::ClientDetails &data);
data::ClientDetails tag_invoke(boost::json::value_to_tag<data::ClientDetails>, const boost::json::value &json_value);

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const data::QueryFederateInput &data);
data::QueryFederateInput tag_invoke(boost::json::value_to_tag<data::QueryFederateInput>,
                                    const boost::json::value &json_value);

} // namespace data