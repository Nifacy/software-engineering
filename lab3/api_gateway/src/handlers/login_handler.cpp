#include <components/credentials_storage.hpp>
#include <handlers/login_handler.hpp>
#include <schemas/auth.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/http/http_status.hpp>

namespace handlers::login_handler {

LoginHandler::LoginHandler(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : common::SchemaHttpHandler(config, context),
      credentials_storage_(
          context.FindComponent<
              components::credentials_storage::CredentialsStorage>()),
      jwt_auth_(
          context.FindComponent<components::jwt_auth::JwtAuthComponent>()) {}

common::Response LoginHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /* context */) const {
  try {
    const auto request_body =
        ParseRequestBody<api_gateway::schemas::auth::LoginRequestBody>(request);

    const auto user_id = credentials_storage_.VerifyCredentials(
        request_body.login, request_body.password);

    const auto token_pair = jwt_auth_.GenerateToken(user_id);
    return common::Response(userver::http::StatusCode::OK,
                            api_gateway::schemas::auth::TokenPair{
                                .accessToken = token_pair.access_token,
                                .refreshToken = token_pair.refresh_token});

  } catch (const components::credentials_storage::CredentialsNotFound&) {
    throw common::HttpError(userver::http::StatusCode::BadRequest,
                            "User with specified login not found");
  } catch (const components::credentials_storage::InvalidVerifySecret&) {
    throw common::HttpError(userver::http::StatusCode::kUnauthorized,
                            "Bad credentials");
  }
}

}  // namespace handlers::login_handler