#include <components/credentials_storage.hpp>
#include <handlers/register_handler.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/http/http_status.hpp>

namespace handlers::register_handler {

RegisterHandler::RegisterHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerBase(config, context),
      storage_(context.FindComponent<
               components::credentials_storage::CredentialsStorage>()) {}

std::string RegisterHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /* context */) const {
  try {
    const auto json = userver::formats::json::FromString(request.RequestBody());
    const std::string credentials = json["credentials"].As<std::string>();

    if (storage_.HasCredentials(credentials)) {
      request.SetResponseStatus(userver::server::http::HttpStatus::Conflict);
      return "User already exists";
    }

    storage_.AddCredentials(credentials);

    request.SetResponseStatus(userver::server::http::HttpStatus::Created);
    return "Created";
  } catch (const userver::formats::json::Exception& e) {
    request.SetResponseStatus(userver::server::http::HttpStatus::BadRequest);
    return "Invalid Payload";
  }
}

}  // namespace handlers::register_handler