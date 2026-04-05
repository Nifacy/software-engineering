#include <handlers/get_current_user_handler.hpp>
#include <schemas/user.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::get_current_user_handler {

GetCurrentUserHandler::GetCurrentUserHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      user_storage_(
          context.FindComponent<components::user_storage::UserStorage>()) {}

common::Response GetCurrentUserHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest&,
    userver::server::request::RequestContext& request_context) const {
  const auto user_id = request_context.GetData<std::string>("user_id");
  const auto user = user_storage_.GetUser(user_id);

  return common::Response(userver::http::StatusCode::OK,
                          api_gateway::schemas::user::UserInfo{
                              .id = user.id,
                              .login = user.login,
                              .firstName = user.firstName,
                              .lastName = user.lastName,
                          });
}

}  // namespace handlers::get_current_user_handler