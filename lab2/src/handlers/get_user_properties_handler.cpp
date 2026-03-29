#include <handlers/get_user_properties_handler.hpp>
#include <schemas/property.hpp>
#include <schemas/user.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::get_user_properties_handler {

api_gateway::schemas::common::Address SerializeAddress(
    const components::property_storage::Address& address) {
  return {
      .country = address.country,
      .city = address.city,
      .street = address.street,
      .building = (std::int32_t)address.building,
      .apartment = (std::int32_t)address.apartment,
  };
}

api_gateway::schemas::property::PropertyStatus SerializePropertyStatus(
    const components::property_storage::PropertyStatus& status) {
  switch (status) {
    case components::property_storage::PropertyStatus::Active:
      return api_gateway::schemas::property::PropertyStatus::kActive;
    case components::property_storage::PropertyStatus::Sold:
      return api_gateway::schemas::property::PropertyStatus::kSold;
  }
}

api_gateway::schemas::user::UserProperty SerializeProperty(
    const std::string& property_id,
    const components::property_storage::Property& property) {
  return {
      .id = property_id,
      .address = SerializeAddress(property.address),
      .status = SerializePropertyStatus(property.status),
      .price = property.price,
  };
}

GetUserPropertiesHandler::GetUserPropertiesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

std::string GetUserPropertiesHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const auto user_id = request_context.GetData<std::string>("user_id");
  api_gateway::schemas::user::UserPropertyList response_dom;

  for (const auto& property_id : property_storage_.FindProperties(
           std::nullopt, std::nullopt, std::nullopt, user_id)) {
    const auto property = property_storage_.GetProperty(property_id);
    response_dom.properties.push_back(SerializeProperty(property_id, property));
  }

  auto response_json =
      userver::formats::json::ValueBuilder{response_dom}.ExtractValue();

  request.SetResponseStatus(userver::server::http::HttpStatus::OK);
  return userver::formats::json::ToString(response_json);
}

}  // namespace handlers::get_user_properties_handler