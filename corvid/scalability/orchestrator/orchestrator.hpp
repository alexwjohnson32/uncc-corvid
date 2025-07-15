#pragma once

#include <string>
#include <vector>
#include <boost/json.hpp>

namespace orchestrator
{
struct HelicsRunner
{
    struct Federate
    {
        std::string directory{};
        std::string exec{};
        std::string host{};
        std::string name{};
    };

    std::string name{};
    std::string logging_path{};
    bool broker{};
    std::vector<Federate> federates{};
};

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const orchestrator::HelicsRunner &data);
orchestrator::HelicsRunner tag_invoke(boost::json::value_to_tag<orchestrator::HelicsRunner>,
                                      const boost::json::value &json_value);

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                const orchestrator::HelicsRunner::Federate &data);
orchestrator::HelicsRunner::Federate tag_invoke(boost::json::value_to_tag<orchestrator::HelicsRunner::Federate>,
                                                const boost::json::value &json_value);
} // namespace orchestrator