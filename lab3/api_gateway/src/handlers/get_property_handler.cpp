#include <handlers/common/utils.hpp>
#include <handlers/get_property_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>

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
  const auto property_id = request.GetPathArg("id");

  try {
    const auto property = property_storage_.GetProperty(property_id);

    return common::Response(
        userver::http::StatusCode::OK,
        api_gateway::schemas::property::Property{
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
            .status = common::SerializePropertyStatus(property.status),
            .price = property.price,
        });
  } catch (const components::property_storage::PropertyNotFound&) {
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "Property with ID '" + property_id + "' not found");
  }
}
}  // namespace handlers::get_property_handler