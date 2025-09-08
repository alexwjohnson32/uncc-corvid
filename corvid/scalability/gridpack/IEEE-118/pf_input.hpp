#pragma once

#include <string>
#include <vector>

#include <boost/json.hpp>

namespace powerflow
{
struct GridlabDInput
{
    std::string name{};
    int bus_id{};
};

struct PowerflowInput
{
    std::string gridpack_name{};
    std::vector<powerflow::GridlabDInput> gridlabd_infos{};
    double total_time{};
    double ln_magnitude{};
};

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const powerflow::GridlabDInput &data);
powerflow::GridlabDInput tag_invoke(boost::json::value_to_tag<powerflow::GridlabDInput>,
                                    const boost::json::value &json_value);

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const powerflow::PowerflowInput &data);
powerflow::PowerflowInput tag_invoke(boost::json::value_to_tag<powerflow::PowerflowInput>,
                                     const boost::json::value &json_value);
} // namespace powerflow