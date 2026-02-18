#include "query_federate_input.hpp"

#include "common/utils/json_templates.hpp"

void queryable::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                           const queryable::ClientDetails &data)
{
    json_value = { { "host", data.host }, { "port", data.port }, { "target", data.target } };
}

queryable::ClientDetails queryable::tag_invoke(boost::json::value_to_tag<queryable::ClientDetails>,
                                               const boost::json::value &json_value)
{
    queryable::ClientDetails data;
    const boost::json::object &obj = json_value.as_object();

    common::utils::extract(obj, "host", data.host);
    common::utils::extract(obj, "port", data.port);
    common::utils::extract(obj, "target", data.target);

    return data;
}

void queryable::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                           const queryable::QueryFederateInput &data)
{
    common::helics::tag_invoke(boost::json::value_from_tag(), json_value,
                               static_cast<const common::helics::HelicsInput &>(data));

    boost::json::object &obj = json_value.as_object();
    obj["client_details"] = boost::json::value_from(data.client_details);
}

queryable::QueryFederateInput queryable::tag_invoke(boost::json::value_to_tag<queryable::QueryFederateInput>,
                                                    const boost::json::value &json_value)
{
    common::helics::HelicsInput base_data = boost::json::value_to<common::helics::HelicsInput>(json_value);

    const boost::json::object &obj = json_value.as_object();
    queryable::QueryFederateInput data{ base_data };

    common::utils::extract(obj, "client_details", data.client_details);

    return data;
}