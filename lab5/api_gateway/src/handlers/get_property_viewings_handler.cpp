#include <components/cache/cache.hpp>
#include <handlers/common/utils.hpp>
#include <handlers/get_property_viewings_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime/date.hpp>
#include <userver/utils/uuid4.hpp>

namespace handlers::get_property_viewings_handler {

GetPropertyViewingsHandler::GetPropertyViewingsHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      viewing_storage_(
          context.FindComponent<components::viewing_storage::ViewingStorage>()),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()),
      viewing_cache_(
          context.FindComponent<components::cache::CacheComponent>().GetCache(
              "viewing_search")) {}

api_gateway::schemas::property::PropertyViewing serializeViewing(
    const boost::uuids::uuid& viewing_id,
    const components::viewing_storage::Viewing& viewing) {
  return {
      .id = userver::utils::ToString(viewing_id),
      .user_id = userver::utils::ToString(viewing.user_id),
      .date = viewing.viewing_date,
  };
}

common::Response GetPropertyViewingsHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /* request_context */) const {
  const auto maybe_property_id =
      handlers::common::TryGetUuidPathArgs(request, "id");
  if (!maybe_property_id.has_value()) {
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "Property not found");
  }

  const auto property_id = *maybe_property_id;
  api_gateway::schemas::property::PropertyViewingList response_dom;

  try {
    property_storage_.GetProperty(property_id);
  } catch (const components::property_storage::PropertyNotFound&) {
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "Property not found");
  }

  for (const auto& [viewing_id, viewing] : FindViewings(property_id)) {
    response_dom.viewings.push_back(serializeViewing(viewing_id, viewing));
  }

  return common::Response(userver::http::StatusCode::OK, response_dom);
}

ViewingList GetPropertyViewingsHandler::FindViewings(
    const boost::uuids::uuid& property_id) const {
  const common::viewings_cache::SearchParams cache_params{
      .entity_type = common::viewings_cache::FilterEntityType::Property,
      .entity_id = property_id,
  };

  const auto cached_result = viewing_cache_.Get(cache_params);
  if (cached_result.has_value()) {
    return cached_result.value();
  }

  ViewingList viewings;
  const auto viewing_ids =
      viewing_storage_.FindViewings(std::nullopt, property_id);
  for (const auto& viewing_id : viewing_ids) {
    const auto viewing = viewing_storage_.GetViewing(viewing_id);
    viewings.push_back({viewing_id, viewing});
  }

  viewing_cache_.Set(cache_params, viewings, std::chrono::seconds{4000});
  return viewings;
}

}  // namespace handlers::get_property_viewings_handler
