#include <handlers/common/utils.hpp>
#include <handlers/get_property_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/boost_uuid4.hpp>

namespace handlers::get_property_handler {

GetPropertyHandler::GetPropertyHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

common::Response GetPropertyHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  const auto maybe_property_id =
      handlers::common::TryGetUuidPathArgs(request, "id");

  if (!maybe_property_id.has_value()) {
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "Property not found");
  }

  const auto property_id = *maybe_property_id;

  try {
    const auto property = property_storage_.GetProperty(property_id);

    return common::Response(
        userver::http::StatusCode::OK,
        api_gateway::schemas::property::Property{
            .id = userver::utils::ToString(property_id),
            .address = handlers::common::SerializeAddress(property.address),
            .ownerId = userver::utils::ToString(property.owner_id),
            .status = common::SerializePropertyStatus(property.status),
            .price = property.price,
        });
  } catch (const components::property_storage::PropertyNotFound&) {
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "Property not found");
  }
}
}  // namespace handlers::get_property_handler