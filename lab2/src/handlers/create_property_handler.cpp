#include <handlers/common/schema_http_handler.hpp>
#include <handlers/create_property_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>
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
  const auto user_id = request_context.GetData<std::string>("user_id");

  const auto property_id = userver::utils::generators::GenerateUuid();
  const components::property_storage::Property new_property{
      .owner_id = user_id,
      .address =
          {
              .country = request_body.address.country,
              .city = request_body.address.city,
              .street = request_body.address.street,
              .building = (unsigned int)request_body.address.building,
              .apartment = (unsigned int)request_body.address.apartment,
          },
      .status = components::property_storage::PropertyStatus::Active,
      .price = request_body.price,
  };

  property_storage_.CreateProperty(property_id, new_property);

  auto user = user_storage_.GetUser(user_id);
  user.propertyIds.emplace(property_id);
  user_storage_.UpdateUser(user);

  return handlers::common::Response(
      userver::server::http::HttpStatus::Created,
      api_gateway::schemas::property::Property{
          .id = property_id,
          .address =
              {
                  .country = new_property.address.country,
                  .city = new_property.address.city,
                  .street = new_property.address.street,
                  .building = (std::int32_t)new_property.address.building,
                  .apartment = (std::int32_t)new_property.address.apartment,
              },
          .ownerId = user_id,
          .status = SerializePropertyStatus(new_property.status),
          .price = new_property.price,
      });
}

}  // namespace handlers::create_property_handler