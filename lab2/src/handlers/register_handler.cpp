#include <components/credentials_storage.hpp>
#include <handlers/register_handler.hpp>
#include <schemas/registration.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/utils/uuid4.hpp>

namespace handlers::register_handler {

RegisterHandler::RegisterHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerBase(config, context),
      credentials_storage_(
          context.FindComponent<
              components::credentials_storage::CredentialsStorage>()),
      user_storage_(
          context.FindComponent<components::user_storage::UserStorage>()) {}

api_gateway::schemas::registration::RegisterRequestBody GetRequestBody(
    const userver::server::http::HttpRequest& request) {
  const auto body_raw = request.RequestBody();
  const auto body_json = userver::formats::json::FromString(body_raw);
  return body_json
      .As<api_gateway::schemas::registration::RegisterRequestBody>();
}

std::string RegisterHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /* context */) const {
  try {
    const auto request_body = GetRequestBody(request);

    if (credentials_storage_.HasCredentials(request_body.login)) {
      request.SetResponseStatus(userver::server::http::HttpStatus::Conflict);
      return "User already exists";
    }

    const auto user_id = userver::utils::generators::GenerateUuid();
    const components::user_storage::User user{user_id, request_body.login,
                                              request_body.firstName,
                                              request_body.lastName};

    user_storage_.CreateUser(user);
    credentials_storage_.AddCredentials(user.id);

    api_gateway::schemas::registration::RegisterResponseBody response_dom{
        user.id};
    auto response_json =
        userver::formats::json::ValueBuilder{response_dom}.ExtractValue();
    request.SetResponseStatus(userver::server::http::HttpStatus::Created);
    return userver::formats::json::ToString(response_json);
  } catch (const userver::formats::json::Exception& e) {
    request.SetResponseStatus(userver::server::http::HttpStatus::BadRequest);
    return "Invalid Payload";
  }
}

}  // namespace handlers::register_handler