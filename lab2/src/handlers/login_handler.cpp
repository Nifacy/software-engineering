#include <components/credentials_storage.hpp>
#include <handlers/login_handler.hpp>
#include <schemas/auth.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/http/http_status.hpp>

namespace handlers::login_handler {

LoginHandler::LoginHandler(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerBase(config, context),
      credentials_storage_(
          context.FindComponent<
              components::credentials_storage::CredentialsStorage>()),
      jwt_auth_(
          context.FindComponent<components::jwt_auth::JwtAuthComponent>()) {}

api_gateway::schemas::auth::LoginRequestBody getRequestBody(
    const userver::server::http::HttpRequest& request) {
  const auto body_raw = request.RequestBody();
  const auto body_json = userver::formats::json::FromString(body_raw);
  return body_json.As<api_gateway::schemas::auth::LoginRequestBody>();
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

std::string LoginHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /* context */) const {
  try {
    const auto request_body = getRequestBody(request);

    const auto user_id = credentials_storage_.VerifyCredentials(
        request_body.login, request_body.password);

    const auto token_pair = jwt_auth_.GenerateToken(user_id);
    request.SetResponseStatus(userver::server::http::HttpStatus::OK);
    return serializeTokenPair(token_pair);
  } catch (const userver::formats::json::Exception& e) {
    request.SetResponseStatus(userver::server::http::HttpStatus::BadRequest);
    return "Invalid Payload";
  } catch (const components::credentials_storage::CredentialsNotFound&) {
    request.SetResponseStatus(userver::server::http::HttpStatus::BadRequest);
    return "User with specified login not found";
  } catch (const components::credentials_storage::InvalidVerifySecret&) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
    return "Bad credentials";
  }
}

}  // namespace handlers::login_handler