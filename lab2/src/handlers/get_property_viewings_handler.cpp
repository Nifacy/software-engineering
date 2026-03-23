#include <handlers/get_property_viewings_handler.hpp>
#include <schemas/property.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/datetime/date.hpp>
#include <userver/utils/uuid4.hpp>

namespace handlers::get_property_viewings_handler {

GetPropertyViewingsHandler::GetPropertyViewingsHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      viewing_storage_(
          context.FindComponent<components::viewing_storage::ViewingStorage>()),
      property_storage_(
          context
              .FindComponent<components::property_storage::PropertyStorage>()) {
}

api_gateway::schemas::property::PropertyViewing SerializeViewing(
    const std::string& viewing_id,
    const components::viewing_storage::Viewing& viewing) {
  return {
      .id = viewing_id,
      .user_id = viewing.user_id,
      .date = userver::utils::datetime::DateFromRFC3339String(viewing.date),
  };
}

std::string GetPropertyViewingsHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /* request_context */) const {
  const auto property_id = request.GetPathArg("id");
  api_gateway::schemas::property::PropertyViewingList response_dom;

  try {
    property_storage_.GetProperty(property_id);
  } catch (const components::property_storage::PropertyNotFound&) {
    request.SetResponseStatus(userver::server::http::HttpStatus::NotFound);
    return "Property with ID '" + property_id + "' not found";
  }

  for (const auto& viewing_id :
       viewing_storage_.FindViewings(std::nullopt, property_id)) {
    const auto viewing = viewing_storage_.GetViewing(viewing_id);
    response_dom.viewings.push_back(SerializeViewing(viewing_id, viewing));
  }

  const auto response_json =
      userver::formats::json::ValueBuilder{response_dom}.ExtractValue();

  request.SetResponseStatus(userver::http::StatusCode::OK);
  return userver::formats::json::ToString(response_json);
}

}  // namespace handlers::get_property_viewings_handler
