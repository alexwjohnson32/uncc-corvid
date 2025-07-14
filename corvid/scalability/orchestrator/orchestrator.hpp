#pragma once

#include <string>
#include <boost/json.hpp>

namespace orchestrator
{
struct CustomObject
{
    std::string m_member;
};

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, orchestrator::CustomObject data);

orchestrator::CustomObject tag_invoke(boost::json::value_to_tag<orchestrator::CustomObject>,
                                      const boost::json::value &json_value);
} // namespace orchestrator