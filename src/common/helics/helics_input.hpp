#pragma once

#include <string>

#include <boost/json.hpp>

namespace common
{
namespace helics
{

struct HelicsInput
{
    std::string federate_name{};
    std::string fed_info_json{};
    double total_time{};
    std::string local_log_file{};
};

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const common::helics::HelicsInput &data);
common::helics::HelicsInput tag_invoke(boost::json::value_to_tag<common::helics::HelicsInput>,
                                       const boost::json::value &json_value);

} // namespace helics
} // namespace common