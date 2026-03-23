#include <handlers/get_property_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::get_property_handler {

GetPropertyHandler::GetPropertyHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

// TODO: move in common part to remove repeats
api_gateway::schemas::property::Property::Status GetPropertyStatus(
    const components::property_storage::PropertyStatus& status) {
  switch (status) {
    case components::property_storage::PropertyStatus::Active:
      return api_gateway::schemas::property::Property::Status::kActive;
    case components::property_storage::PropertyStatus::Sold:
      return api_gateway::schemas::property::Property::Status::kSold;
  }
}

std::string GetPropertyHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  const auto property_id = request.GetPathArg("id");

  try {
    const auto property = property_storage_.GetProperty(property_id);
    const api_gateway::schemas::property::Property response_dom{
        .id = property_id,
        .address =
            {
                .country = property.address.country,
                .city = property.address.city,
                .street = property.address.street,
                .building = (std::int32_t)property.address.building,
                .apartment = (std::int32_t)property.address.apartment,
            },
        .ownerId = property.owner_id,
        .status = GetPropertyStatus(property.status),
        .price = property.price,
    };

    request.SetResponseStatus(userver::server::http::HttpStatus::OK);
    auto response_json =
        userver::formats::json::ValueBuilder{response_dom}.ExtractValue();

    return userver::formats::json::ToString(response_json);

  } catch (const components::property_storage::PropertyNotFound&) {
    request.SetResponseStatus(userver::server::http::HttpStatus::NotFound);
    return "Property with ID '" + property_id + "' not found";
  }
}
}  // namespace handlers::get_property_handler