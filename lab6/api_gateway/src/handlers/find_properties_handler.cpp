#include <components/cache/cache.hpp>
#include <handlers/find_properties_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/boost_uuid4.hpp>

namespace handlers::find_properties_handler {

struct FindPropertiesHandler::FindParams {
  std::optional<std::string> city_pattern;
  std::optional<int> min_price;
  std::optional<int> max_price;
};

FindPropertiesHandler::FindPropertiesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()),
      property_cache_(
          context.FindComponent<components::cache::CacheComponent>().GetCache(
              "property_search")) {}

std::optional<int> FindPropertiesHandler::TryGetIntArg(
    const userver::server::http::HttpRequest& request,
    const std::string& arg_name, const std::optional<int> minimum) const {
  const auto maybe_value = TryGetArg(request, arg_name);
  if (!maybe_value.has_value()) {
    return std::nullopt;
  }

  const std::string value = *maybe_value;

  try {
    const auto parsedValue = std::stoi(value);

    if (minimum.has_value() && parsedValue < minimum.value()) {
      throw common::HttpError(userver::http::StatusCode::BadRequest,
                              "Value for argument '" + arg_name +
                                  "' must be not lower than " +
                                  std::to_string(minimum.value()));
    }

    return parsedValue;

  } catch (const std::exception& e) {
    throw common::HttpError(
        userver::http::StatusCode::BadRequest,
        "Query argument '" + arg_name + "' must be an integer");
  }
}

std::vector<std::string> serializePropertyIds(
    const std::vector<boost::uuids::uuid>& property_ids) {
  std::vector<std::string> result;

  for (const auto& el : property_ids) {
    result.push_back(userver::utils::ToString(el));
  }

  return result;
}

common::Response FindPropertiesHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  const auto property_ids = FindProperties({
      .city_pattern = TryGetArg(request, "city"),
      .min_price = TryGetIntArg(request, "minPrice", 0),
      .max_price = TryGetIntArg(request, "maxPrice", 0),
  });

  return common::Response(
      userver::http::StatusCode::OK,
      api_gateway::schemas::property::FindPropertiesResponse{
          .propertyIds = serializePropertyIds(property_ids),
      });
}

std::vector<boost::uuids::uuid> FindPropertiesHandler::FindProperties(
    const FindParams& params) const {
  const auto cached_property_ids =
      property_cache_.Get({.city_pattern = params.city_pattern,
                           .min_price = params.min_price,
                           .max_price = params.max_price});

  if (cached_property_ids.has_value()) {
    return cached_property_ids.value();
  }

  const auto property_ids = property_storage_.FindProperties(
      params.city_pattern, params.min_price, params.max_price, std::nullopt);

  property_cache_.Set(
      {
          .city_pattern = params.city_pattern,
          .min_price = params.min_price,
          .max_price = params.max_price,
      },
      property_ids, std::chrono::seconds{200});

  return property_ids;
}

}  // namespace handlers::find_properties_handler