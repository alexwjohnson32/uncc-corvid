#include "helics_input.hpp"
#include "common/utils/json_templates.hpp"

void common::helics::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                                const common::helics::HelicsInput &data)
{
    json_value = { { "federate_name", data.federate_name },
                   { "fed_info_json", boost::json::parse(data.fed_info_json) },
                   { "total_time", data.total_time },
                   { "local_log_file", data.local_log_file } };
}

common::helics::HelicsInput common::helics::tag_invoke(boost::json::value_to_tag<common::helics::HelicsInput>,
                                                       const boost::json::value &json_value)
{
    common::helics::HelicsInput data;
    const boost::json::object &obj = json_value.as_object();

    common::utils::extract(obj, "federate_name", data.federate_name);
    common::utils::extract(obj, "fed_info_json", common::utils::raw_json(data.fed_info_json));
    common::utils::extract(obj, "total_time", data.total_time);
    common::utils::extract(obj, "local_log_file", data.local_log_file);

    return data;
}