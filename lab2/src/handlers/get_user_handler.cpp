#include <handlers/get_user_handler.hpp>
#include <schemas/user.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::get_user_handler {

GetUserHandler::GetUserHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      user_storage_(
          context.FindComponent<components::user_storage::UserStorage>()) {}

std::string GetUserHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /* request_context */) const {
  const auto user_id = request.GetPathArg("id");

  try {
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
  } catch (components::user_storage::UserNotFound) {
    request.SetResponseStatus(userver::server::http::HttpStatus::NotFound);
    return "User with ID '" + user_id + "' not found";
  }
}

}  // namespace handlers::get_user_handler