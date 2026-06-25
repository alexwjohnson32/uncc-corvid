#pragma once

#include "common/helics/helics_input.hpp"

#include <vector>
#include <string>

#include <boost/json.hpp>

namespace one_phase
{
struct GridlabDInputs
{
    int bus_id{};
    std::vector<std::string> names{};
};

struct PhaseInput : public common::helics::HelicsInput
{
    std::vector<one_phase::GridlabDInputs> gridlabd_infos{};
    double ln_magnitude{};
    std::string phase_name{};
    std::string publication_field{};
    std::string subscription_field{};
    std::string xml_file{};
    std::string raw_file{};
    double rotation_degrees{};

    std::vector<std::string> GetGridlabDNames() const;
};

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const one_phase::GridlabDInputs &data);
one_phase::GridlabDInputs tag_invoke(boost::json::value_to_tag<one_phase::GridlabDInputs>,
                                     const boost::json::value &json_value);

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const one_phase::PhaseInput &data);
one_phase::PhaseInput tag_invoke(boost::json::value_to_tag<one_phase::PhaseInput>,
                                 const boost::json::value &json_value);
} // namespace one_phase