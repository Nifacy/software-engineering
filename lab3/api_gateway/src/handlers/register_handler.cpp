#include <components/credentials_storage/credentials_storage.hpp>
#include <handlers/register_handler.hpp>
#include <schemas/auth.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/utils/uuid4.hpp>

namespace handlers::register_handler {

RegisterHandler::RegisterHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : common::SchemaHttpHandler(config, context),
      credentials_storage_(
          context.FindComponent<
              components::credentials_storage::CredentialsStorage>()),
      user_storage_(
          context.FindComponent<components::user_storage::UserStorage>()),
      jwt_auth_(
          context.FindComponent<components::jwt_auth::JwtAuthComponent>()) {}

common::Response RegisterHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  try {
    const auto request_body =
        ParseRequestBody<api_gateway::schemas::auth::RegisterRequestBody>(
            request);
    const auto user_id = userver::utils::generators::GenerateUuid();

    const components::credentials_storage::Credentials credentials{
        .verify_secret = request_body.password,
        .payload = user_id,
    };
    credentials_storage_.AddCredentials(request_body.login, credentials);

    const components::user_storage::User user{
        .id = user_id,
        .login = request_body.login,
        .firstName = request_body.firstName,
        .lastName = request_body.lastName,
        .propertyIds = std::unordered_set<std::string>(),
    };

    user_storage_.CreateUser(user);

    const auto token_pair = jwt_auth_.GenerateToken(user_id);
    return common::Response(userver::http::StatusCode::Created,
                            api_gateway::schemas::auth::TokenPair{
                                .accessToken = token_pair.access_token,
                                .refreshToken = token_pair.refresh_token,
                            });

  } catch (const components::credentials_storage::CredentialsAlreadyExists&) {
    throw common::HttpError(userver::http::StatusCode::Conflict,
                            "User already exists");
  }
}

}  // namespace handlers::register_handler