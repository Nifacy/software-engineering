#pragma once
#include <handlers/common/schema_http_handler.hpp>
#include <rate_limiter/sliding_window_counter.hpp>

namespace handlers::common {

class ThrottledHttpHandler : public SchemaHttpHandler {
 public:
  ThrottledHttpHandler(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);

  static userver::yaml_config::Schema GetStaticConfigSchema();

 protected:
  Response HandleRequestImpl(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

  virtual Response HandleLimitedRequest(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const = 0;

 private:
  rate_limiter::SlidingWindowCounter& rate_limiter_;
};

}  // namespace handlers::common