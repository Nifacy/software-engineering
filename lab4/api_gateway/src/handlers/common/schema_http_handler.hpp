#pragma once

#include <components/property_storage/property_storage.hpp>
#include <components/viewing_storage/viewing_storage.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::common {

class Response final {
 public:
  template <typename T>
  Response(const userver::server::http::HttpStatus& status, const T& content)
      : status_code_(status),
        content_(userver::formats::json::ToString(
            userver::formats::json::ValueBuilder{content}.ExtractValue())) {}

  Response(const userver::server::http::HttpStatus& status);
  void ApplyToRequest(const userver::server::http::HttpRequest&, std::string&);

 private:
  userver::server::http::HttpStatus status_code_;
  std::optional<std::string> content_;
};

class HttpError : std::runtime_error {
 public:
  HttpError(const userver::server::http::HttpStatus&, const std::string&);

  void ApplyToRequest(const userver::server::http::HttpRequest&,
                      std::string&) const;

 private:
  userver::server::http::HttpStatus status_code_;
  std::string error_message_;
};

class SchemaHttpHandler : public userver::server::handlers::HttpHandlerBase {
 public:
  SchemaHttpHandler(const userver::components::ComponentConfig& config,
                    const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const final override;

 protected:
  virtual Response HandleRequestImpl(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const = 0;

  std::optional<std::string> TryGetArg(
      const userver::server::http::HttpRequest&, const std::string&) const;

  template <typename T>
  T ParseRequestBody(const userver::server::http::HttpRequest& request) const {
    try {
      const auto body_raw = request.RequestBody();
      const auto body_json = userver::formats::json::FromString(body_raw);
      return body_json.As<T>();
    } catch (const std::exception&) {
      throw HttpError(userver::http::StatusCode::BadRequest, "Invalid Payload");
    }
  }
};

}  // namespace handlers::common
