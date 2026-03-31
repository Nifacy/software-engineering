#include <handlers/schedule_viewing_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>
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

common::Response ScheduleViewingHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const auto property_id = request.GetPathArg("id");
  const auto user_id = request_context.GetData<std::string>("user_id");
  const auto request_body = ParseRequestBody<
      api_gateway::schemas::property::ScheduleViewingRequestBody>(request);

  try {
    const auto property = property_storage_.GetProperty(property_id);

    if (property.status !=
        components::property_storage::PropertyStatus::Active) {
      throw common::HttpError(
          userver::http::StatusCode::BadRequest,
          "Viewing can be scheduled only for active properties");
    }

    const auto viewing_id = userver::utils::generators::GenerateUuid();
    const auto viewing_date =
        userver::utils::datetime::ToString(request_body.date);

    for (const auto& viewing_id :
         viewing_storage_.FindViewings(std::nullopt, property_id)) {
      const auto viewing = viewing_storage_.GetViewing(viewing_id);
      if (viewing.date == viewing_date) {
        if (viewing.user_id == user_id) {
          return common::Response(userver::http::StatusCode::kNotModified);
        } else {
          throw common::HttpError(userver::http::StatusCode::BadRequest,
                                  "Viewing already scheduled for this date");
        }
      }
    }

    const components::viewing_storage::Viewing new_viewing{
        .user_id = user_id,
        .property_id = property_id,
        .date = userver::utils::datetime::ToString(request_body.date),
    };

    viewing_storage_.CreateViewing(viewing_id, new_viewing);

    return common::Response(
        userver::http::StatusCode::Created,
        api_gateway::schemas::property::PropertyViewing{
            .id = viewing_id,
            .user_id = new_viewing.user_id,
            .date = userver::utils::datetime::DateFromRFC3339String(
                new_viewing.date),
        });
  } catch (const components::property_storage::PropertyNotFound&) {
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "Property with ID '" + property_id + "' not found");
  }
}

}  // namespace handlers::schedule_viewing_handler
