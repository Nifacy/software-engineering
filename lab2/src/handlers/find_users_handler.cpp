#include <handlers/find_users_handler.hpp>
#include <schemas/user.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::find_users_handler {

FindUsersHandler::FindUsersHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      user_storage_(
          context.FindComponent<components::user_storage::UserStorage>()) {}

std::optional<std::string> TryGetFilterValue(
    const userver::server::http::HttpRequest& request,
    const std::string& arg_name) {
  const auto value = request.GetArg(arg_name);
  if (value.empty()) {
    return std::nullopt;
  }
  return value;
}

std::string FindUsersHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  const auto user_ids =
      user_storage_.FindUsers(TryGetFilterValue(request, "login"),
                              TryGetFilterValue(request, "firstName"),
                              TryGetFilterValue(request, "lastName"));

  const api_gateway::schemas::user::UserIds response_dom{.userIds = user_ids};
  auto response_json =
      userver::formats::json::ValueBuilder{response_dom}.ExtractValue();

  request.SetResponseStatus(userver::server::http::HttpStatus::OK);
  return userver::formats::json::ToString(response_json);
}

}  // namespace handlers::find_users_handler