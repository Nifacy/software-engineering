#include <handlers/schedule_viewing_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/datetime/date.hpp>
#include <userver/utils/uuid4.hpp>

namespace handlers::schedule_viewing_handler {

ScheduleViewingHandler::ScheduleViewingHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      viewing_storage_(
          context.FindComponent<components::viewing_storage::ViewingStorage>()),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

api_gateway::schemas::property::ScheduleViewingRequestBody GetRequestBody(
    const userver::server::http::HttpRequest& request) {
  const auto body_raw = request.RequestBody();
  const auto body_json = userver::formats::json::FromString(body_raw);
  return body_json
      .As<api_gateway::schemas::property::ScheduleViewingRequestBody>();
}

std::string SerializeViewing(
    const std::string& viewing_id,
    const components::viewing_storage::Viewing& viewing) {
  const api_gateway::schemas::property::PropertyViewing response_dom{
      .id = viewing_id,
      .user_id = viewing.user_id,
      .date = userver::utils::datetime::DateFromRFC3339String(viewing.date),
  };

  auto response_json =
      userver::formats::json::ValueBuilder{response_dom}.ExtractValue();

  return userver::formats::json::ToString(response_json);
}

std::string ScheduleViewingHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const auto property_id = request.GetPathArg("id");
  const auto user_id = request_context.GetData<std::string>("user_id");
  const auto request_body = GetRequestBody(request);

  try {
    const auto property = property_storage_.GetProperty(property_id);

    if (property.status !=
        components::property_storage::PropertyStatus::Active) {
      request.SetResponseStatus(userver::server::http::HttpStatus::BadRequest);
      return "Viewing can be scheduled only for active properties";
    }

    const auto viewing_id = userver::utils::generators::GenerateUuid();
    const auto viewing_date =
        userver::utils::datetime::ToString(request_body.date);

    for (const auto& viewing_id :
         viewing_storage_.FindViewings(std::nullopt, property_id)) {
      const auto viewing = viewing_storage_.GetViewing(viewing_id);
      if (viewing.date == viewing_date) {
        if (viewing.user_id == user_id) {
          request.SetResponseStatus(
              userver::server::http::HttpStatus::kNotModified);
          return "";
        } else {
          request.SetResponseStatus(
              userver::server::http::HttpStatus::BadRequest);
          return "Viewing already scheduled for this date";
        }
      }
    }

    const components::viewing_storage::Viewing new_viewing{
        .user_id = user_id,
        .property_id = property_id,
        .date = userver::utils::datetime::ToString(request_body.date),
    };

    viewing_storage_.CreateViewing(viewing_id, new_viewing);

    request.SetResponseStatus(userver::server::http::HttpStatus::Created);
    return SerializeViewing(viewing_id, new_viewing);
  } catch (const components::property_storage::PropertyNotFound&) {
    request.SetResponseStatus(userver::server::http::HttpStatus::NotFound);
    return "Property with ID '" + property_id + "' not found";
  }
}

}  // namespace handlers::schedule_viewing_handler
