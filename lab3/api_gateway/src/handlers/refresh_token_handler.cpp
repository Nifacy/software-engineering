#include <handlers/refresh_token_handler.hpp>
#include <schemas/auth.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::refresh_token_handler {

RefreshTokenHandler::RefreshTokenHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      jwt_auth_(
          context.FindComponent<components::jwt_auth::JwtAuthComponent>()) {}

common::Response RefreshTokenHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  const auto request_body =
      ParseRequestBody<api_gateway::schemas::auth::RefreshTokenRequestBody>(
          request);

  try {
    const auto token_pair = jwt_auth_.RefreshToken(request_body.accessToken);
    return common::Response(userver::http::StatusCode::OK,
                            api_gateway::schemas::auth::TokenPair{
                                .accessToken = token_pair.access_token,
                                .refreshToken = token_pair.refresh_token,
                            });
  } catch (const components::jwt_auth::InvalidToken&) {
    throw common::HttpError(userver::http::StatusCode::BadRequest,
                            "Invalid token");
  }
}

}  // namespace handlers::refresh_token_handler