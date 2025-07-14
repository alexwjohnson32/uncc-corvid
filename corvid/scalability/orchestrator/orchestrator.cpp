#include "orchestrator.hpp"

#include "JsonTemplates.hpp"

void orchestrator::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                              orchestrator::CustomObject data)
{
    json_value = { { "member", data.m_member } };
}

orchestrator::CustomObject orchestrator::tag_invoke(boost::json::value_to_tag<orchestrator::CustomObject>,
                                                    const boost::json::value &json_value)
{
    orchestrator::CustomObject data;
    const boost::json::object &obj = json_value.as_object();

    json_templates::extract(obj, "member", data.m_member);

    return data;
}