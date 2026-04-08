#include <handlers/common/utils.hpp>
#include <handlers/delete_viewing_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime/date.hpp>
#include <userver/utils/uuid4.hpp>

namespace handlers::delete_viewing_handler {

DeleteViewingHandler::DeleteViewingHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      viewing_storage_(
          context.FindComponent<components::viewing_storage::ViewingStorage>()),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

std::optional<components::viewing_storage::Viewing> TryGetViewing(
    const components::viewing_storage::ViewingStorage& viewing_storage,
    const boost::uuids::uuid& property_id,
    const boost::uuids::uuid& viewing_id) {
  try {
    const auto viewing = viewing_storage.GetViewing(viewing_id);
    if (viewing.property_id == property_id) {
      return viewing;
    } else {
      return std::nullopt;
    }
  } catch (const components::viewing_storage::ViewingNotFound&) {
    return std::nullopt;
  }
}

handlers::common::Response DeleteViewingHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const auto property_id =
      handlers::common::TryGetUuidPathArgs(request, "property_id");
  const auto user_id = request_context.GetData<boost::uuids::uuid>("user_id");
  const auto viewing_id =
      handlers::common::TryGetUuidPathArgs(request, "viewing_id");

  if (!property_id.has_value() || !viewing_id.has_value()) {
    throw handlers::common::HttpError(
        userver::server::http::HttpStatus::NotFound, "Viewing not found");
  }

  const auto maybe_viewing =
      TryGetViewing(viewing_storage_, *property_id, *viewing_id);

  if (!maybe_viewing.has_value()) {
    throw handlers::common::HttpError(
        userver::server::http::HttpStatus::NotFound, "Viewing not found");
  }

  if ((*maybe_viewing).user_id != user_id) {
    throw handlers::common::HttpError(
        userver::server::http::HttpStatus::kForbidden,
        "You don't have permission to delete this viewing");
  }

  viewing_storage_.DeleteViewing(*viewing_id);
  return handlers::common::Response(
      userver::server::http::HttpStatus::NoContent);
}

}  // namespace handlers::delete_viewing_handler
