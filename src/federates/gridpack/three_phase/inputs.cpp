#include "inputs.hpp"
#include "app.hpp"
#include "common/utils/json_templates.hpp"

void three_phase::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                             const three_phase::GridlabDInputs &data)
{
    json_value = { { "bus_id", data.bus_id }, { "names", data.names } };
}

three_phase::GridlabDInputs three_phase::tag_invoke(boost::json::value_to_tag<three_phase::GridlabDInputs>,
                                                    const boost::json::value &json_value)
{
    three_phase::GridlabDInputs data;
    const boost::json::object &obj = json_value.as_object();

    common::utils::extract(obj, "bus_id", data.bus_id);
    common::utils::extract(obj, "names", data.names);

    return data;
}

void three_phase::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                             const three_phase::PhaseInput &data)
{
    json_value = { { "xml_file", data.xml_file },
                   { "raw_file", data.raw_file },
                   { "rotation_degrees", data.rotation_degrees } };
}

three_phase::PhaseInput three_phase::tag_invoke(boost::json::value_to_tag<three_phase::PhaseInput>,
                                                const boost::json::value &json_value)
{
    three_phase::PhaseInput data;
    const boost::json::object &obj = json_value.as_object();

    common::utils::extract(obj, "xml_file", data.xml_file);
    common::utils::extract(obj, "raw_file", data.raw_file);
    common::utils::extract(obj, "rotation_degrees", data.rotation_degrees);

    return data;
}

void three_phase::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                             const three_phase::ThreePhaseInput &data)
{
    common::helics::tag_invoke(boost::json::value_from_tag(), json_value,
                               static_cast<const common::helics::HelicsInput &>(data));

    boost::json::object &obj = json_value.as_object();
    obj["gridlabd_infos"] = boost::json::value_from(data.gridlabd_infos);
    obj["ln_magnitude"] = data.ln_magnitude;
    obj["phase_a"] = boost::json::value_from(data.phase_a);
    obj["phase_b"] = boost::json::value_from(data.phase_b);
    obj["phase_c"] = boost::json::value_from(data.phase_c);
}

three_phase::ThreePhaseInput three_phase::tag_invoke(boost::json::value_to_tag<three_phase::ThreePhaseInput>,
                                                     const boost::json::value &json_value)
{
    common::helics::HelicsInput base_data = boost::json::value_to<common::helics::HelicsInput>(json_value);

    const boost::json::object &obj = json_value.as_object();
    three_phase::ThreePhaseInput data{ base_data };

    common::utils::extract(obj, "gridlabd_infos", data.gridlabd_infos);
    common::utils::extract(obj, "ln_magnitude", data.ln_magnitude);
    common::utils::extract(obj, "phase_a", data.phase_a);
    common::utils::extract(obj, "phase_b", data.phase_b);
    common::utils::extract(obj, "phase_c", data.phase_c);

    return data;
}

std::vector<std::string> three_phase::ThreePhaseInput::GetGridlabDNames() const
{
    std::vector<std::string> names;

    for (const three_phase::GridlabDInputs &info : gridlabd_infos)
    {
        for (const std::string &name : info.names)
        {
            names.push_back(name);
        }
    }

    return names;
}