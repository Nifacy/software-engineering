#include <handlers/get_user_handler.hpp>
#include <schemas/user.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::get_user_handler {

GetUserHandler::GetUserHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      user_storage_(
          context.FindComponent<components::user_storage::UserStorage>()) {}

common::Response GetUserHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /* request_context */) const {
  const auto user_id = request.GetPathArg("id");

  try {
    const auto user = user_storage_.GetUser(user_id);
    return common::Response(userver::http::StatusCode::OK,
                            api_gateway::schemas::user::UserInfo{
                                .id = user.id,
                                .login = user.login,
                                .firstName = user.firstName,
                                .lastName = user.lastName,
                            });
  } catch (const components::user_storage::UserNotFound&) {
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "User with ID '" + user_id + "' not found");
  }
}

}  // namespace handlers::get_user_handler