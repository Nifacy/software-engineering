#include <components/rate_limiter/rate_limiter.hpp>

namespace components::rate_limiter {

RateLimiterComponent::RateLimiterComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context), limiters_() {}

::rate_limiter::SlidingWindowCounter& RateLimiterComponent::GetLimiter(
    const std::string& key, const RateLimiterConfig& config) {
  if (limiters_.contains(key)) {
    return *limiters_.at(key);
  }

  limiters_.emplace(key, CreateLimiter(config));
  return *limiters_.at(key);
}

SlidingWindowPtr RateLimiterComponent::CreateLimiter(
    const RateLimiterConfig& config) {
  return std::make_shared<::rate_limiter::SlidingWindowCounter>(
      config.max_requests, std::chrono::seconds(config.window_size));
}

}  // namespace components::rate_limiter