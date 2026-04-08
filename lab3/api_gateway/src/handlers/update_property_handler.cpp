#include <handlers/common/utils.hpp>
#include <handlers/update_property_handler.hpp>
#include <iostream>
#include <schemas/common.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/boost_uuid4.hpp>

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
  const auto user_id = request_context.GetData<boost::uuids::uuid>("user_id");
  const auto maybe_property_id =
      handlers::common::TryGetUuidPathArgs(request, "id");

  const auto request_body = ParseRequestBody<
      api_gateway::schemas::property::UpdatePropertyRequestBody>(request);

  if (!maybe_property_id.has_value()) {
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "Property not found");
  }

  const auto property_id = *maybe_property_id;

  try {
    auto property = property_storage_.GetProperty(property_id);
    if (property.owner_id != user_id) {
      throw common::HttpError(
          userver::http::StatusCode::kForbidden,
          "You don't have permission to update this property");
    }

    const auto maybe_property_status =
        request_body.status.has_value()
            ? std::optional(DeserializePropertyStatus(*request_body.status))
            : std::nullopt;

    property_storage_.UpdateProperty(property_id, maybe_property_status,
                                     request_body.price);

    property = property_storage_.GetProperty(property_id);

    return common::Response(
        userver::http::StatusCode::OK,
        api_gateway::schemas::property::Property{
            .id = userver::utils::ToString(property_id),
            .address = common::SerializeAddress(property.address),
            .ownerId = userver::utils::ToString(property.owner_id),
            .status = common::SerializePropertyStatus(property.status),
            .price = property.price,
        });

  } catch (const components::property_storage::PropertyNotFound&) {
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "Property not found");
  }
}

}  // namespace handlers::update_property_handler
