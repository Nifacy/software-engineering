#include <handlers/update_property_handler.hpp>
#include <schemas/common.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::update_property_handler {

api_gateway::schemas::property::UpdatePropertyRequestBody GetRequestBody(
    const userver::server::http::HttpRequest& request) {
  const auto body_raw = request.RequestBody();
  const auto body_json = userver::formats::json::FromString(body_raw);
  return body_json
      .As<api_gateway::schemas::property::UpdatePropertyRequestBody>();
}

components::property_storage::PropertyStatus DeserializePropertyStatus(
    const api_gateway::schemas::property::PropertyStatus& status) {
  switch (status) {
    case api_gateway::schemas::property::PropertyStatus::kActive:
      return components::property_storage::PropertyStatus::Active;
    case api_gateway::schemas::property::PropertyStatus::kSold:
      return components::property_storage::PropertyStatus::Sold;
  }
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

api_gateway::schemas::property::Property SerializeProperty(
    const std::string& property_id,
    const components::property_storage::Property& property) {
  return {
      .id = property_id,
      .address = SerializeAddress(property.address),
      .ownerId = property.owner_id,
      .status = SerializePropertyStatus(property.status),
      .price = property.price,
  };
}

UpdatePropertyHandler::UpdatePropertyHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

// FIXME: This method returns 500 status code on request validation error
std::string UpdatePropertyHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const auto user_id = request_context.GetData<std::string>("user_id");
  const auto property_id = request.GetPathArg("id");
  const auto request_body = GetRequestBody(request);

  try {
    auto property = property_storage_.GetProperty(property_id);
    if (property.owner_id != user_id) {
      request.SetResponseStatus(userver::http::StatusCode::kForbidden);
      return "You don't have permission to update this property";
    }

    if (request_body.price.has_value()) {
      property.price = *request_body.price;
    }

    if (request_body.status.has_value()) {
      property.status = DeserializePropertyStatus(*request_body.status);
    }

    property_storage_.UpdateProperty(property_id, property);

    request.SetResponseStatus(userver::http::StatusCode::OK);

    const auto response_dom = SerializeProperty(property_id, property);
    const auto response_json =
        userver::formats::json::ValueBuilder{response_dom}.ExtractValue();
    return userver::formats::json::ToString(response_json);

  } catch (const components::property_storage::PropertyNotFound&) {
    request.SetResponseStatus(userver::http::StatusCode::NotFound);
    return "Property with ID '" + property_id + "' not found";
  }
}

}  // namespace handlers::update_property_handler
