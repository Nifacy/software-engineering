#include <handlers/common/schema_http_handler.hpp>
#include <handlers/create_property_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/uuid4.hpp>

namespace handlers::create_property_handler {

using CreatePropertyRequestBody =
    api_gateway::schemas::property::CreatePropertyRequestBody;

using PropertyStatus = api_gateway::schemas::property::PropertyStatus;

CreatePropertyHandler::CreatePropertyHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      user_storage_(
          context.FindComponent<components::user_storage::UserStorage>()),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

PropertyStatus SerializePropertyStatus(
    const components::property_storage::PropertyStatus& status) {
  switch (status) {
    case components::property_storage::PropertyStatus::Active:
      return PropertyStatus::kActive;
    case components::property_storage::PropertyStatus::Sold:
      return PropertyStatus::kSold;
  }
}

handlers::common::Response CreatePropertyHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const auto request_body =
      ParseRequestBody<CreatePropertyRequestBody>(request);
  const auto user_id = request_context.GetData<boost::uuids::uuid>("user_id");

  const components::property_storage::Property new_property{
      .owner_id = user_id,
      .address =
          {
              .country = request_body.address.country,
              .city = request_body.address.city,
              .street = request_body.address.street,
              .building = request_body.address.building,
              .apartment = request_body.address.apartment,
          },
      .status = components::property_storage::PropertyStatus::Active,
      .price = request_body.price,
  };

  const auto property_id = property_storage_.CreateProperty(new_property);

  // TODO: remove this
  const auto apartment = new_property.address.apartment.has_value()
                             ? *new_property.address.apartment
                             : 0;

  return handlers::common::Response(
      userver::server::http::HttpStatus::Created,
      api_gateway::schemas::property::Property{
          .id = userver::utils::ToString(property_id),
          .address =
              {
                  .country = new_property.address.country,
                  .city = new_property.address.city,
                  .street = new_property.address.street,
                  .building = (std::int32_t)new_property.address.building,
                  .apartment = (std::int32_t)apartment,
              },
          .ownerId = userver::utils::ToString(user_id),
          .status = SerializePropertyStatus(new_property.status),
          .price = new_property.price,
      });
}

}  // namespace handlers::create_property_handler