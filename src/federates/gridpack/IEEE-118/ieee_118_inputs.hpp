#pragma once

#include "helics_input.hpp"

#include <vector>
#include <string>

#include <boost/json.hpp>

namespace ieee_118
{
struct GridlabDInputs
{
    int bus_id{};
    std::vector<std::string> names{};
};

struct IEEE118Input : public common::helics::HelicsInput
{
    std::vector<ieee_118::GridlabDInputs> gridlabd_infos{};
    double ln_magnitude;

    std::vector<std::string> GetGridalabDNames() const;
};

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const ieee_118::GridlabDInputs &data);
ieee_118::GridlabDInputs tag_invoke(boost::json::value_to_tag<ieee_118::GridlabDInputs>,
                                    const boost::json::value &json_value);

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const ieee_118::IEEE118Input &data);
ieee_118::IEEE118Input tag_invoke(boost::json::value_to_tag<ieee_118::IEEE118Input>,
                                  const boost::json::value &json_value);
} // namespace ieee_118