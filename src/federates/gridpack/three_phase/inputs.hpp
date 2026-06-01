#pragma once

#include "common/helics/helics_input.hpp"

#include <vector>
#include <string>

#include <boost/json.hpp>

namespace three_phase
{
struct GridlabDInputs
{
    int bus_id{};
    std::vector<std::string> names{};
};

struct PhaseInput
{
    std::string xml_file{};
    std::string raw_file{};
    double rotation_degrees{};
};

struct ThreePhaseInput : public common::helics::HelicsInput
{
    std::vector<three_phase::GridlabDInputs> gridlabd_infos{};
    double ln_magnitude{};
    PhaseInput phase_a{};
    PhaseInput phase_b{};
    PhaseInput phase_c{};

    std::vector<std::string> GetGridlabDNames() const;
};

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const three_phase::GridlabDInputs &data);
three_phase::GridlabDInputs tag_invoke(boost::json::value_to_tag<three_phase::GridlabDInputs>,
                                       const boost::json::value &json_value);

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const three_phase::PhaseInput &data);
three_phase::PhaseInput tag_invoke(boost::json::value_to_tag<three_phase::PhaseInput>,
                                   const boost::json::value &json_value);

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const three_phase::ThreePhaseInput &data);
three_phase::ThreePhaseInput tag_invoke(boost::json::value_to_tag<three_phase::ThreePhaseInput>,
                                        const boost::json::value &json_value);
} // namespace three_phase