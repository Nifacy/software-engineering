#include <handlers/get_current_user_handler.hpp>
#include <schemas/user.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::get_current_user_handler {

GetCurrentUserHandler::GetCurrentUserHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      user_storage_(
          context.FindComponent<components::user_storage::UserStorage>()) {}

std::string GetCurrentUserHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& /* request */,
    userver::server::request::RequestContext& request_context) const {
  const auto user_id = request_context.GetData<std::string>("user_id");
  const auto user = user_storage_.GetUser(user_id);

  const api_gateway::schemas::user::UserInfo response_dom{
      .id = user.id,
      .login = user.login,
      .firstName = user.firstName,
      .lastName = user.lastName,
  };

  auto response_json =
      userver::formats::json::ValueBuilder{response_dom}.ExtractValue();
  return userver::formats::json::ToString(response_json);
}

}  // namespace handlers::get_current_user_handler