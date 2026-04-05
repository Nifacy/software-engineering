#include <handlers/find_properties_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::find_properties_handler {

FindPropertiesHandler::FindPropertiesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

std::optional<int> FindPropertiesHandler::TryGetIntArg(
    const userver::server::http::HttpRequest& request,
    const std::string& arg_name) const {
  const auto maybe_value = TryGetArg(request, arg_name);
  if (!maybe_value.has_value()) {
    return std::nullopt;
  }

  int result = 0;
  const std::string value = *maybe_value;
  auto [ptr, ec] =
      std::from_chars(value.data(), value.data() + value.size(), result);

  if (ec == std::errc() && ptr == value.data() + value.size()) {
    return result;
  }

  throw common::HttpError(
      userver::http::StatusCode::BadRequest,
      "Query argument '" + arg_name + "' must be an integer");
}

common::Response FindPropertiesHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  const auto property_ids = property_storage_.FindProperties(
      TryGetArg(request, "city"), TryGetIntArg(request, "minPrice"),
      TryGetIntArg(request, "maxPrice"), std::nullopt);

  return common::Response(
      userver::http::StatusCode::OK,
      api_gateway::schemas::property::FindPropertiesResponse{
          .propertyIds = property_ids,
      });
}

}  // namespace handlers::find_properties_handler