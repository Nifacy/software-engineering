#include <handlers/get_user_viewings_handler.hpp>
#include <schemas/user.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/datetime/date.hpp>

namespace handlers::get_user_viewings_handler {

GetUserViewingsHandler::GetUserViewingsHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      viewing_storage_(
          context
              .FindComponent<components::viewing_storage::ViewingStorage>()) {}

api_gateway::schemas::user::UserViewing SerializeViewing(
    const std::string& viewing_id,
    const components::viewing_storage::Viewing& viewing) {
  return {
      .propertyId = viewing.property_id,
      .id = viewing_id,
      .date = userver::utils::datetime::DateFromRFC3339String(viewing.date),
  };
}

std::string GetUserViewingsHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const auto user_id = request_context.GetData<std::string>("user_id");
  api_gateway::schemas::user::UserViewingList response_dom;

  for (const auto& viewing_id :
       viewing_storage_.FindViewings(user_id, std::nullopt)) {
    const auto viewing = viewing_storage_.GetViewing(viewing_id);
    response_dom.viewings.push_back(SerializeViewing(viewing_id, viewing));
  }

  const auto response_json =
      userver::formats::json::ValueBuilder{response_dom}.ExtractValue();

  request.SetResponseStatus(userver::server::http::HttpStatus::OK);
  return userver::formats::json::ToString(response_json);
}

}  // namespace handlers::get_user_viewings_handler