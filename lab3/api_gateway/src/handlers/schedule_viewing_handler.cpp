#include <handlers/common/utils.hpp>
#include <handlers/schedule_viewing_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime/date.hpp>
#include <userver/utils/uuid4.hpp>

namespace handlers::schedule_viewing_handler {

ScheduleViewingHandler::ScheduleViewingHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      viewing_storage_(
          context.FindComponent<components::viewing_storage::ViewingStorage>()),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

bool isTakenByCurrentUser(components::viewing_storage::ViewingStorage& storage,
                          const boost::uuids::uuid& user_id,
                          const boost::uuids::uuid& property_id,
                          const userver::utils::datetime::Date& viewing_date) {
  const auto viewings_ids =
      storage.FindViewings(user_id, std::optional(property_id));

  for (const auto& viewing_id : viewings_ids) {
    const auto viewing = storage.GetViewing(viewing_id);
    if (viewing.viewing_date == viewing_date) {
      return true;
    }
  }

  return false;
}

common::Response ScheduleViewingHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const auto maybe_property_id =
      handlers::common::TryGetUuidPathArgs(request, "id");
  const auto user_id = request_context.GetData<boost::uuids::uuid>("user_id");
  const auto request_body = ParseRequestBody<
      api_gateway::schemas::property::ScheduleViewingRequestBody>(request);

  if (!maybe_property_id.has_value()) {
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "Property not found");
  }

  const auto property_id = *maybe_property_id;

  try {
    const auto property = property_storage_.GetProperty(property_id);

    if (property.status !=
        components::property_storage::PropertyStatus::Active) {
      throw common::HttpError(
          userver::http::StatusCode::BadRequest,
          "Viewing can be scheduled only for active properties");
    }

    const components::viewing_storage::Viewing new_viewing{
        .user_id = user_id,
        .property_id = property_id,
        .viewing_date = request_body.date,
    };

    const auto viewing_id = viewing_storage_.CreateViewing(new_viewing);

    return common::Response(
        userver::http::StatusCode::Created,
        api_gateway::schemas::property::PropertyViewing{
            .id = userver::utils::ToString(viewing_id),
            .user_id = userver::utils::ToString(new_viewing.user_id),
            .date = new_viewing.viewing_date,
        });
  } catch (const components::viewing_storage::ViewingDateTaken&) {
    if (isTakenByCurrentUser(viewing_storage_, user_id, property_id,
                             request_body.date)) {
      return common::Response(userver::http::StatusCode::kNotModified);
    } else {
      throw common::HttpError(userver::http::StatusCode::BadRequest,
                              "Viewing date already taken");
    }
  } catch (const components::property_storage::PropertyNotFound&) {
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "Property not found");
  }
}

}  // namespace handlers::schedule_viewing_handler
