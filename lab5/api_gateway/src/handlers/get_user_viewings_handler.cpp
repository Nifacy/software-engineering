#include <components/cache/cache.hpp>
#include <handlers/get_user_viewings_handler.hpp>
#include <schemas/user.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime/date.hpp>

namespace handlers::get_user_viewings_handler {

api_gateway::schemas::user::UserViewing serializeViewing(
    const boost::uuids::uuid& viewing_id,
    const components::viewing_storage::Viewing& viewing) {
  return {
      .propertyId = userver::utils::ToString(viewing.property_id),
      .id = userver::utils::ToString(viewing_id),
      .date = viewing.viewing_date,
  };
}

GetUserViewingsHandler::GetUserViewingsHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      viewing_storage_(
          context.FindComponent<components::viewing_storage::ViewingStorage>()),
      viewing_cache_(
          context.FindComponent<components::cache::CacheComponent>().GetCache(
              "viewing_search")) {}

common::Response GetUserViewingsHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest&,
    userver::server::request::RequestContext& request_context) const {
  const auto user_id = request_context.GetData<boost::uuids::uuid>("user_id");
  api_gateway::schemas::user::UserViewingList response_dom;

  for (const auto& [viewing_id, viewing] : FindViewings(user_id)) {
    // TODO: move in common module
    response_dom.viewings.push_back(serializeViewing(viewing_id, viewing));
  }

  return common::Response(userver::http::StatusCode::OK, response_dom);
}

ViewingList GetUserViewingsHandler::FindViewings(
    const boost::uuids::uuid& user_id) const {
  const common::viewings_cache::SearchParams cache_params{
      .entity_type = common::viewings_cache::FilterEntityType::User,
      .entity_id = user_id,
  };

  const auto cached_result = viewing_cache_.Get(cache_params);
  if (cached_result.has_value()) {
    return cached_result.value();
  }

  ViewingList viewings;
  const auto viewing_ids = viewing_storage_.FindViewings(user_id, std::nullopt);
  for (const auto& viewing_id : viewing_ids) {
    const auto viewing = viewing_storage_.GetViewing(viewing_id);
    viewings.push_back({viewing_id, viewing});
  }

  viewing_cache_.Set(cache_params, viewings, std::chrono::seconds{4000});
  return viewings;
}

}  // namespace handlers::get_user_viewings_handler