#include <handlers/find_properties_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::find_properties_handler {

class InvalidArgValue : std::runtime_error {
 public:
  InvalidArgValue(const std::string& arg_name)
      : std::runtime_error("Query argument '" + arg_name +
                           "' must be an integer") {}
  std::string GetMessage() const { return std::string(what()); }
};

FindPropertiesHandler::FindPropertiesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

std::optional<std::string> TryGetFilterValue(
    const userver::server::http::HttpRequest& request,
    const std::string& arg_name) {
  const auto value = request.GetArg(arg_name);
  if (value.empty()) {
    return std::nullopt;
  }
  return value;
}

std::optional<int> TryGetIntegerFilterValue(
    const userver::server::http::HttpRequest& request,
    const std::string& arg_name) {
  const auto maybe_value = TryGetFilterValue(request, arg_name);
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

  throw InvalidArgValue(arg_name);
}

std::string FindPropertiesHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  try {
    const auto property_ids = property_storage_.FindProperties(
        TryGetFilterValue(request, "city"),
        TryGetIntegerFilterValue(request, "minPrice"),
        TryGetIntegerFilterValue(request, "maxPrice"), std::nullopt);

    const api_gateway::schemas::property::FindPropertiesResponse response_dom{
        .propertyIds = property_ids};
    auto response_json =
        userver::formats::json::ValueBuilder{response_dom}.ExtractValue();

    request.SetResponseStatus(userver::server::http::HttpStatus::OK);
    return userver::formats::json::ToString(response_json);
  } catch (const InvalidArgValue& e) {
    request.SetResponseStatus(userver::server::http::HttpStatus::BadRequest);
    return e.GetMessage();
  }
}

}  // namespace handlers::find_properties_handler