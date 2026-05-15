#pragma once

#include <cache/interface.hpp>
#include <memory>
#include <rate_limiter/sliding_window_counter.hpp>
#include <string>
#include <userver/components/component_base.hpp>

namespace components::rate_limiter {

struct RateLimiterConfig {
  int max_requests;
  int window_size;
};

using SlidingWindowPtr = std::shared_ptr<::rate_limiter::SlidingWindowCounter>;

class RateLimiterComponent final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "rate-limiter";

  explicit RateLimiterComponent(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

  ::rate_limiter::SlidingWindowCounter& GetLimiter(
      const std::string& key, const RateLimiterConfig& config);

 private:
  SlidingWindowPtr CreateLimiter(const RateLimiterConfig& config);
  std::unordered_map<std::string, SlidingWindowPtr> limiters_;
};

}  // namespace components::rate_limiter