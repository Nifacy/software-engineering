#include <handlers/common/utils.hpp>
#include <handlers/get_user_properties_handler.hpp>
#include <schemas/property.hpp>
#include <schemas/user.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/boost_uuid4.hpp>

namespace handlers::get_user_properties_handler {

GetUserPropertiesHandler::GetUserPropertiesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

common::Response GetUserPropertiesHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest&,
    userver::server::request::RequestContext& request_context) const {
  const auto user_id = request_context.GetData<boost::uuids::uuid>("user_id");
  api_gateway::schemas::user::UserPropertyList response_dom;

  for (const auto& property_id : property_storage_.FindProperties(
           std::nullopt, std::nullopt, std::nullopt, user_id)) {
    const auto property = property_storage_.GetProperty(property_id);
    response_dom.properties.push_back({
        .id = userver::utils::ToString(property_id),
        .address = common::SerializeAddress(property.address),
        .status = common::SerializePropertyStatus(property.status),
        .price = property.price,
    });
  }

  return common::Response(userver::http::StatusCode::OK, response_dom);
}

}  // namespace handlers::get_user_properties_handler