#include "pf_input.hpp"

#include "json_templates.hpp"

void powerflow::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                           const powerflow::GridlabDInputs &data)
{
    json_value = { { "bus_id", data.bus_id }, { "names", data.names } };
}

powerflow::GridlabDInputs powerflow::tag_invoke(boost::json::value_to_tag<powerflow::GridlabDInputs>,
                                                const boost::json::value &json_value)
{
    powerflow::GridlabDInputs data;
    const boost::json::object &obj = json_value.as_object();

    utils::extract(obj, "bus_id", data.bus_id);
    utils::extract(obj, "names", data.names);

    return data;
}

void powerflow::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                           const powerflow::PowerflowInput &data)
{
    json_value = { { "gridpack_name", data.gridpack_name },
                   { "fed_info_json", boost::json::parse(data.fed_info_json) },
                   { "gridlabd_infos", data.gridlabd_infos },
                   { "total_time", data.total_time },
                   { "ln_magnitude", data.ln_magnitude } };
}

powerflow::PowerflowInput powerflow::tag_invoke(boost::json::value_to_tag<powerflow::PowerflowInput>,
                                                const boost::json::value &json_value)
{
    powerflow::PowerflowInput data;
    const boost::json::object &obj = json_value.as_object();

    utils::extract(obj, "gridpack_name", data.gridpack_name);
    utils::extract_json_string(obj, "fed_info_json", data.fed_info_json);
    utils::extract(obj, "gridlabd_infos", data.gridlabd_infos);
    utils::extract(obj, "total_time", data.total_time);
    utils::extract(obj, "ln_magnitude", data.ln_magnitude);

    return data;
}

std::vector<std::string> powerflow::PowerflowInput::GetGridalabDNames() const
{
    std::vector<std::string> names;

    for (const powerflow::GridlabDInputs &info : gridlabd_infos)
    {
        for (const std::string &name : info.names)
        {
            names.push_back(name);
        }
    }

    return names;
}