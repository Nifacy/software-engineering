#include <handlers/create_property_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/uuid4.hpp>

namespace handlers::create_property_handler {

CreatePropertyHandler::CreatePropertyHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      user_storage_(
          context.FindComponent<components::user_storage::UserStorage>()),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

api_gateway::schemas::property::CreatePropertyRequestBody GetRequestBody(
    const userver::server::http::HttpRequest& request) {
  const auto body_raw = request.RequestBody();
  const auto body_json = userver::formats::json::FromString(body_raw);
  return body_json
      .As<api_gateway::schemas::property::CreatePropertyRequestBody>();
}

api_gateway::schemas::property::Property::Status GetPropertyStatus(
    const components::property_storage::PropertyStatus& status) {
  switch (status) {
    case components::property_storage::PropertyStatus::Active:
      return api_gateway::schemas::property::Property::Status::kActive;
    case components::property_storage::PropertyStatus::Sold:
      return api_gateway::schemas::property::Property::Status::kSold;
  }
}

std::string CreatePropertyHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const auto request_body = GetRequestBody(request);
  const auto user_id = request_context.GetData<std::string>("user_id");

  const auto property_id = userver::utils::generators::GenerateUuid();
  const components::property_storage::Property new_property{
      .owner_id = user_id,
      .address =
          {
              .country = request_body.address.city,
              .city = request_body.address.city,
              .street = request_body.address.street,
              .building = (unsigned int)request_body.address.building,
              .apartment = (unsigned int)request_body.address.apartment,
          },
      .status = components::property_storage::PropertyStatus::Active,
  };

  property_storage_.CreateProperty(property_id, new_property);

  auto user = user_storage_.GetUser(user_id);
  user.propertyIds.emplace(property_id);
  user_storage_.UpdateUser(user);

  const api_gateway::schemas::property::Property response_dom{
      .id = property_id,
      .address =
          {
              .country = new_property.address.city,
              .city = new_property.address.city,
              .street = new_property.address.street,
              .building = (std::int32_t)new_property.address.building,
              .apartment = (std::int32_t)new_property.address.apartment,
          },
      .ownerId = user_id,
      .status = GetPropertyStatus(new_property.status),
  };

  request.SetResponseStatus(userver::server::http::HttpStatus::Created);
  auto response_json =
      userver::formats::json::ValueBuilder{response_dom}.ExtractValue();
  return userver::formats::json::ToString(response_json);
}

}  // namespace handlers::create_property_handler