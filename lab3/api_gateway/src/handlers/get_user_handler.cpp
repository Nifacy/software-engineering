#include <handlers/common/utils.hpp>
#include <handlers/get_user_handler.hpp>
#include <schemas/user.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/boost_uuid4.hpp>

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
  const auto maybe_user_id =
      handlers::common::TryGetUuidPathArgs(request, "id");
  if (!maybe_user_id.has_value()) {
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "User not found");
  }

  const auto user_id = *maybe_user_id;

  try {
    const auto user = user_storage_.GetUser(user_id);
    return common::Response(userver::http::StatusCode::OK,
                            api_gateway::schemas::user::UserInfo{
                                .id = userver::utils::ToString(user_id),
                                .login = user.login,
                                .firstName = user.first_name,
                                .lastName = user.last_name,
                            });
  } catch (const components::user_storage::UserNotFound&) {
    // TODO: DRY
    throw common::HttpError(userver::http::StatusCode::NotFound,
                            "User not found");
  }
}

}  // namespace handlers::get_user_handler