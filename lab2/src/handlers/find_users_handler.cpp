#include <handlers/find_users_handler.hpp>
#include <schemas/user.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::find_users_handler {

FindUsersHandler::FindUsersHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      user_storage_(
          context.FindComponent<components::user_storage::UserStorage>()) {}

common::Response FindUsersHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  const auto user_ids = user_storage_.FindUsers(TryGetArg(request, "login"),
                                                TryGetArg(request, "firstName"),
                                                TryGetArg(request, "lastName"));

  return common::Response(userver::http::StatusCode::OK,
                          api_gateway::schemas::user::UserIds{
                              .userIds = user_ids,
                          });
}

}  // namespace handlers::find_users_handler