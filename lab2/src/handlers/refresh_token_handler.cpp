#include <handlers/refresh_token_handler.hpp>
#include <schemas/auth.hpp>
#include <userver/components/component_context.hpp>

namespace handlers::refresh_token_handler {

api_gateway::schemas::auth::RefreshTokenRequestBody getRequestBody(
    const userver::server::http::HttpRequest& request) {
  const auto body_raw = request.RequestBody();
  const auto body_json = userver::formats::json::FromString(body_raw);
  return body_json.As<api_gateway::schemas::auth::RefreshTokenRequestBody>();
}

std::string serializeTokenPair(
    const components::jwt_auth::TokenPair& token_pair) {
  const api_gateway::schemas::auth::TokenPair response_dom{
      .accessToken = token_pair.access_token,
      .refreshToken = token_pair.refresh_token,
  };

  auto response_json =
      userver::formats::json::ValueBuilder{response_dom}.ExtractValue();

  return userver::formats::json::ToString(response_json);
}

RefreshTokenHandler::RefreshTokenHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      jwt_auth_(
          context.FindComponent<components::jwt_auth::JwtAuthComponent>()) {}

std::string RefreshTokenHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  const auto request_body = getRequestBody(request);

  try {
    const auto token_pair = jwt_auth_.RefreshToken(request_body.accessToken);
    request.SetResponseStatus(userver::http::StatusCode::OK);
    return serializeTokenPair(token_pair);
  } catch (const components::jwt_auth::InvalidToken&) {
    request.SetResponseStatus(userver::http::StatusCode::BadRequest);
    return "Invalid token";
  }
}

}  // namespace handlers::refresh_token_handler