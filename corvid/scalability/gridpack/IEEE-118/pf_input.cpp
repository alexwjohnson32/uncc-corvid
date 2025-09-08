#include "pf_input.hpp"

#include "JsonTemplates.hpp"

#include <boost/json.hpp>

void powerflow::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                           const powerflow::GridlabDInput &data)
{
    json_value = { { "name", data.name }, { "bus_id", data.bus_id } };
}

powerflow::GridlabDInput powerflow::tag_invoke(boost::json::value_to_tag<powerflow::GridlabDInput>,
                                               const boost::json::value &json_value)
{
    powerflow::GridlabDInput data;
    const boost::json::object &obj = json_value.as_object();

    json_templates::extract(obj, "name", data.name);
    json_templates::extract(obj, "bus_id", data.bus_id);

    return data;
}

void powerflow::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                           const powerflow::PowerflowInput &data)
{
    json_value = { { "gridpack_name", data.gridpack_name },
                   { "gridlabd_infos", data.gridlabd_infos },
                   { "total_time", data.total_time },
                   { "ln_magnitude", data.ln_magnitude } };
}

powerflow::PowerflowInput powerflow::tag_invoke(boost::json::value_to_tag<powerflow::PowerflowInput>,
                                                const boost::json::value &json_value)
{
    powerflow::PowerflowInput data;
    const boost::json::object &obj = json_value.as_object();

    json_templates::extract(obj, "gridpack_name", data.gridpack_name);
    json_templates::extract(obj, "gridlabd_infos", data.gridlabd_infos);
    json_templates::extract(obj, "total_time", data.total_time);
    json_templates::extract(obj, "ln_magnitude", data.ln_magnitude);

    return data;
}