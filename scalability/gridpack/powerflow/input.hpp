#pragma once

#include <string>
#include <vector>

#include <boost/json.hpp>

namespace powerflow
{
namespace input
{

struct GridlabDInputs
{
    int bus_id{};
    std::vector<std::string> names{};
};

struct PowerflowInput
{
    std::string gridpack_name{};
    std::string fed_info_json{};
    std::vector<GridlabDInputs> gridlabd_infos{};
    double total_time{};
    double ln_magnitude{};

    std::vector<std::string> GetGridalabDNames() const;
};

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const GridlabDInputs &data);
GridlabDInputs tag_invoke(boost::json::value_to_tag<GridlabDInputs>, const boost::json::value &json_value);

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const PowerflowInput &data);
PowerflowInput tag_invoke(boost::json::value_to_tag<PowerflowInput>, const boost::json::value &json_value);

} // namespace input
} // namespace powerflow