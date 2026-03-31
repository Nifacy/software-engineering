#include <handlers/common/utils.hpp>
#include <handlers/update_property_handler.hpp>
#include <schemas/common.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::update_property_handler {

components::property_storage::PropertyStatus DeserializePropertyStatus(
    const api_gateway::schemas::property::PropertyStatus& status) {
  switch (status) {
    case api_gateway::schemas::property::PropertyStatus::kActive:
      return components::property_storage::PropertyStatus::Active;
    case api_gateway::schemas::property::PropertyStatus::kSold:
      return components::property_storage::PropertyStatus::Sold;
  }

  throw std::runtime_error("Unknown property status");
}

UpdatePropertyHandler::UpdatePropertyHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

common::Response UpdatePropertyHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const auto user_id = request_context.GetData<std::string>("user_id");
  const auto property_id = request.GetPathArg("id");
  const auto request_body = ParseRequestBody<
      api_gateway::schemas::property::UpdatePropertyRequestBody>(request);

  try {
    auto property = property_storage_.GetProperty(property_id);
    if (property.owner_id != user_id) {
      throw common::HttpError(
          userver::http::StatusCode::kForbidden,
          "You don't have permission to update this property");
    }

    if (request_body.price.has_value()) {
      property.price = *request_body.price;
    }

    if (request_body.status.has_value()) {
      property.status = DeserializePropertyStatus(*request_body.status);
    }

    property_storage_.UpdateProperty(property_id, property);

    request.SetResponseStatus(userver::http::StatusCode::OK);

    return common::Response(
        userver::http::StatusCode::OK,
        api_gateway::schemas::property::Property{
            .id = property_id,
            .address = common::SerializeAddress(property.address),
            .ownerId = property.owner_id,
            .status = common::SerializePropertyStatus(property.status),
            .price = property.price,
        });

  } catch (const components::property_storage::PropertyNotFound&) {
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "Property with ID '" + property_id + "' not found");
  }
}

}  // namespace handlers::update_property_handler
