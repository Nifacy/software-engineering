#include <handlers/info_handler.hpp>

namespace handlers::info_handler {

CredentialsInfoHandler::CredentialsInfoHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string CredentialsInfoHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& /* request */,
    userver::server::request::RequestContext& request_context) const {
  return "Credentials: " + request_context.GetData<std::string>("credentials") +
         "\n";
}

}  // namespace handlers::info_handler