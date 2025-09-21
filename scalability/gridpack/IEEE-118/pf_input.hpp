#pragma once

#include <string>
#include <vector>

#include <boost/json.hpp>

namespace powerflow
{
struct GridlabDInputs
{
    int bus_id{};
    std::vector<std::string> names{};
};

struct PowerflowInput
{
    std::string gridpack_name{};
    std::vector<powerflow::GridlabDInputs> gridlabd_infos{};
    double total_time{};
    double ln_magnitude{};

    std::vector<std::string> GetGridalabDNames() const;
};

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const powerflow::GridlabDInputs &data);
powerflow::GridlabDInputs tag_invoke(boost::json::value_to_tag<powerflow::GridlabDInputs>,
                                    const boost::json::value &json_value);

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const powerflow::PowerflowInput &data);
powerflow::PowerflowInput tag_invoke(boost::json::value_to_tag<powerflow::PowerflowInput>,
                                     const boost::json::value &json_value);
} // namespace powerflow