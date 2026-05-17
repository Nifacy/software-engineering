#include <handlers/common/schema_http_handler.hpp>

namespace handlers::common {

Response::Response(const userver::http::StatusCode& status_code)
    : status_code_(status_code), content_(std::nullopt), response_headers_() {}

void Response::ApplyToRequest(const userver::server::http::HttpRequest& request,
                              std::string& response_body) {
  auto& response = request.GetHttpResponse();
  request.SetResponseStatus(status_code_);
  response.SetContentType("application/json");
  response_body = content_.has_value() ? *content_ : "";

  for (const auto& [key, value] : response_headers_) {
    response.SetHeader(key, value);
  }
}

void Response::AddHeader(const std::string& key, const std::string& value) {
  response_headers_[key] = value;
}

HttpError::HttpError(const userver::server::http::HttpStatus& status_code,
                     const std::string& message)
    : std::runtime_error("Http Error"),
      status_code_(status_code),
      error_message_(message) {}

void HttpError::ApplyToRequest(
    const userver::server::http::HttpRequest& request,
    std::string& response_body) const {
  response_body = error_message_;
  request.SetResponseStatus(status_code_);
}

SchemaHttpHandler::SchemaHttpHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::optional<std::string> SchemaHttpHandler::TryGetArg(
    const userver::server::http::HttpRequest& request,
    const std::string& arg_name) const {
  const auto value = request.GetArg(arg_name);
  return value.empty() ? std::nullopt : std::optional(value);
}

std::string SchemaHttpHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  std::string response_body;

  try {
    auto response = HandleRequestImpl(request, request_context);
    response.ApplyToRequest(request, response_body);
    return response_body;
  } catch (const HttpError& error) {
    error.ApplyToRequest(request, response_body);
    return response_body;
  } catch (const std::exception& error) {
    LOG_ERROR() << "Unexpected error in '" << this->HandlerName() << "' handler: " << error.what();
    throw error;
  }
}

}  // namespace handlers::common