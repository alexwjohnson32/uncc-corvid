#include "inputs.hpp"
#include "app.hpp"
#include "common/utils/json_templates.hpp"

void one_phase::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                           const one_phase::GridlabDInputs &data)
{
    json_value = { { "bus_id", data.bus_id }, { "names", data.names } };
}

one_phase::GridlabDInputs one_phase::tag_invoke(boost::json::value_to_tag<one_phase::GridlabDInputs>,
                                                const boost::json::value &json_value)
{
    one_phase::GridlabDInputs data;
    const boost::json::object &obj = json_value.as_object();

    common::utils::extract(obj, "bus_id", data.bus_id);
    common::utils::extract(obj, "names", data.names);

    return data;
}

void one_phase::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                           const one_phase::PhaseInput &data)
{
    common::helics::tag_invoke(boost::json::value_from_tag(), json_value,
                               static_cast<const common::helics::HelicsInput &>(data));

    boost::json::object &obj = json_value.as_object();
    obj["gridlabd_infos"] = boost::json::value_from(data.gridlabd_infos);
    obj["ln_magnitude"] = data.ln_magnitude;
    obj["phase_name"] = data.phase_name;
    obj["publication_field"] = data.publication_field;
    obj["subscription_field"] = data.subscription_field;
    obj["xml_file"] = data.xml_file;
    obj["raw_file"] = data.raw_file;
    obj["rotation_degrees"] = data.rotation_degrees;
}

one_phase::PhaseInput one_phase::tag_invoke(boost::json::value_to_tag<one_phase::PhaseInput>,
                                            const boost::json::value &json_value)
{
    common::helics::HelicsInput base_data = boost::json::value_to<common::helics::HelicsInput>(json_value);

    const boost::json::object &obj = json_value.as_object();
    one_phase::PhaseInput data{ base_data };

    common::utils::extract(obj, "gridlabd_infos", data.gridlabd_infos);
    common::utils::extract(obj, "ln_magnitude", data.ln_magnitude);
    common::utils::extract(obj, "phase_name", data.phase_name);
    common::utils::extract(obj, "publication_field", data.publication_field);
    common::utils::extract(obj, "subscription_field", data.subscription_field);
    common::utils::extract(obj, "xml_file", data.xml_file);
    common::utils::extract(obj, "raw_file", data.raw_file);
    common::utils::extract(obj, "rotation_degrees", data.rotation_degrees);

    return data;
}

std::vector<std::string> one_phase::PhaseInput::GetGridlabDNames() const
{
    std::vector<std::string> names;

    for (const one_phase::GridlabDInputs &info : gridlabd_infos)
    {
        for (const std::string &name : info.names)
        {
            names.push_back(name);
        }
    }

    return names;
}