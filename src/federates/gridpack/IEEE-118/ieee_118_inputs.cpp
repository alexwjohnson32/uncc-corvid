#include "ieee_118_inputs.hpp"
#include "json_templates.hpp"

void ieee_118::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                          const ieee_118::GridlabDInputs &data)
{
    json_value = { { "bus_id", data.bus_id }, { "names", data.names } };
}

ieee_118::GridlabDInputs ieee_118::tag_invoke(boost::json::value_to_tag<ieee_118::GridlabDInputs>,
                                              const boost::json::value &json_value)
{
    ieee_118::GridlabDInputs data;
    const boost::json::object &obj = json_value.as_object();

    common::utils::extract(obj, "bus_id", data.bus_id);
    common::utils::extract(obj, "names", data.names);

    return data;
}

void ieee_118::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                          const ieee_118::IEEE118Input &data)
{
    common::helics::tag_invoke(boost::json::value_from_tag(), json_value,
                               static_cast<const common::helics::HelicsInput &>(data));

    boost::json::object &obj = json_value.as_object();
    obj["gridlabd_infos"] = boost::json::value_from(data.gridlabd_infos);
    obj["ln_magnitude"] = data.ln_magnitude;
}

ieee_118::IEEE118Input ieee_118::tag_invoke(boost::json::value_to_tag<ieee_118::IEEE118Input>,
                                            const boost::json::value &json_value)
{
    common::helics::HelicsInput base_data = boost::json::value_to<common::helics::HelicsInput>(json_value);

    const boost::json::object &obj = json_value.as_object();
    ieee_118::IEEE118Input data{ base_data };

    common::utils::extract(obj, "gridlabd_infos", data.gridlabd_infos);
    common::utils::extract(obj, "ln_magnitude", data.ln_magnitude);

    return data;
}

std::vector<std::string> ieee_118::IEEE118Input::GetGridalabDNames() const
{
    std::vector<std::string> names;

    for (const ieee_118::GridlabDInputs &info : gridlabd_infos)
    {
        for (const std::string &name : info.names)
        {
            names.push_back(name);
        }
    }

    return names;
}