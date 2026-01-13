#include "query_federate_input.hpp"

#include "json_templates.hpp"

void data::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const data::ClientDetails &data)
{
    json_value = { { "host", data.host }, { "port", data.port }, { "target", data.target } };
}

data::ClientDetails data::tag_invoke(boost::json::value_to_tag<data::ClientDetails>,
                                     const boost::json::value &json_value)
{
    data::ClientDetails data;
    const boost::json::object &obj = json_value.as_object();

    utils::extract(obj, "host", data.host);
    utils::extract(obj, "port", data.port);
    utils::extract(obj, "target", data.target);

    return data;
}

void data::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const data::QueryFederateInput &data)
{
    json_value = { { "federate_name", data.federate_name },
                   { "fed_info_json", boost::json::parse(data.fed_info_json) },
                   { "client_details", data.client_details },
                   { "total_time", data.total_time },
                   { "local_log_file", data.local_log_file } };
}

data::QueryFederateInput data::tag_invoke(boost::json::value_to_tag<data::QueryFederateInput>,
                                          const boost::json::value &json_value)
{
    data::QueryFederateInput data;
    const boost::json::object &obj = json_value.as_object();

    utils::extract(obj, "federate_name", data.federate_name);
    utils::extract_json_string(obj, "fed_info_json", data.fed_info_json);
    utils::extract(obj, "client_details", data.client_details);
    utils::extract(obj, "total_time", data.total_time);
    utils::extract(obj, "local_log_file", data.local_log_file);

    return data;
}