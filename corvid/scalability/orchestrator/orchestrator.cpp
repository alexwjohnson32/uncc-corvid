#include "orchestrator.hpp"

#include "JsonTemplates.hpp"

void orchestrator::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                              const orchestrator::HelicsRunner &data)
{
    json_value = { { "name", data.name },
                   { "logging_path", data.logging_path },
                   { "broker", data.broker },
                   { "federates", data.federates } };
}

orchestrator::HelicsRunner orchestrator::tag_invoke(boost::json::value_to_tag<orchestrator::HelicsRunner>,
                                                    const boost::json::value &json_value)
{
    orchestrator::HelicsRunner data;
    const boost::json::object &obj = json_value.as_object();

    json_templates::extract(obj, "name", data.name);
    json_templates::extract(obj, "logging_path", data.logging_path);
    json_templates::extract(obj, "broker", data.broker);
    json_templates::extract(obj, "federates", data.federates);

    return data;
}

void orchestrator::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                              const orchestrator::HelicsRunner::Federate &data)
{
    json_value = {
        { "directory", data.directory }, { "exec", data.exec }, { "host", data.host }, { "name", data.name }
    };
}

orchestrator::HelicsRunner::Federate
orchestrator::tag_invoke(boost::json::value_to_tag<orchestrator::HelicsRunner::Federate>,
                         const boost::json::value &json_value)
{
    orchestrator::HelicsRunner::Federate data;
    const boost::json::object &obj = json_value.as_object();

    json_templates::extract(obj, "directory", data.directory);
    json_templates::extract(obj, "exec", data.exec);
    json_templates::extract(obj, "host", data.host);
    json_templates::extract(obj, "name", data.name);

    return data;
}

void orchestrator::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                              const orchestrator::HelicsRunner::Broker &data)
{
    json_value = { { "coreType", data.core_type }, { "initString", data.init_string } };
}

orchestrator::HelicsRunner::Broker
orchestrator::tag_invoke(boost::json::value_to_tag<orchestrator::HelicsRunner::Broker>,
                         const boost::json::value &json_value)
{
    orchestrator::HelicsRunner::Broker data;
    const boost::json::object &obj = json_value.as_object();

    json_templates::extract(obj, "coreType", data.core_type);
    json_templates::extract(obj, "initString", data.init_string);

    return data;
}