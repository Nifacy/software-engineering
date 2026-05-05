#include <components/rate_limiter/rate_limiter.hpp>
#include <handlers/common/throttled_http_handler.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace handlers::common {

ThrottledHttpHandler::ThrottledHttpHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      rate_limiter_(
          context
              .FindComponent<components::rate_limiter::RateLimiterComponent>()
              .GetLimiter(config["limiter_key"].As<std::string>(),
                          {
                              .max_requests = config["max_requests"].As<int>(),
                              .window_size = config["window_size"].As<int>(),
                          })) {}

userver::yaml_config::Schema ThrottledHttpHandler::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<SchemaHttpHandler>(
      R"(
type: object
additionalProperties: false
description: HTTP handler with rate limiting
properties:
  max_requests:
    type: integer
    description: maximum amount of requests per second
  window_size:
    type: integer
    description: size of windows in seconds
  limiter_key:
    type: string
    description: unique key of rate limiter
)");
}

Response ThrottledHttpHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
  auto limit_result = rate_limiter_.TryAcquire();
  auto response = limit_result.allowed
                      ? HandleLimitedRequest(request, context)
                      : Response(userver::http::StatusCode::TooManyRequests,
                                 "Too many requests");
  response.AddHeader("X-RateLimit-Limit", "unknown");
  response.AddHeader("X-RateLimit-Remaining", "unknown");
  response.AddHeader("X-RateLimit-Reset", "unknown");
  return response;
}

}  // namespace handlers::common