#include <components/credentials_storage.hpp>
#include <handlers/register_handler.hpp>
#include <schemas/auth.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/utils/uuid4.hpp>

namespace handlers::register_handler {

api_gateway::schemas::auth::RegisterRequestBody getRequestBody(
    const userver::server::http::HttpRequest& request) {
  const auto body_raw = request.RequestBody();
  const auto body_json = userver::formats::json::FromString(body_raw);
  return body_json.As<api_gateway::schemas::auth::RegisterRequestBody>();
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

RegisterHandler::RegisterHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerBase(config, context),
      credentials_storage_(
          context.FindComponent<
              components::credentials_storage::CredentialsStorage>()),
      user_storage_(
          context.FindComponent<components::user_storage::UserStorage>()),
      jwt_auth_(
          context.FindComponent<components::jwt_auth::JwtAuthComponent>()) {}

std::string RegisterHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /* context */) const {
  try {
    const auto request_body = getRequestBody(request);
    const auto user_id = userver::utils::generators::GenerateUuid();

    const components::credentials_storage::Credentials credentials{
        .verify_secret = request_body.password,  // TODO: hash password
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
    request.SetResponseStatus(userver::server::http::HttpStatus::Created);
    return serializeTokenPair(token_pair);
  } catch (const userver::formats::json::Exception& e) {
    request.SetResponseStatus(userver::server::http::HttpStatus::BadRequest);
    return "Invalid Payload";
  } catch (const components::credentials_storage::CredentialsAlreadyExists&) {
    request.SetResponseStatus(userver::server::http::HttpStatus::Conflict);
    return "User already exists";
  }
}

}  // namespace handlers::register_handler