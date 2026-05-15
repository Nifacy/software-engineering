#include <cache/in_memory.hpp>
#include <components/cache/cache.hpp>
#include <userver/components/component_config.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace components::cache {

CacheComponent::CacheComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      caches_(),
      cache_max_size_(config["max_size"].As<size_t>()) {}

userver::yaml_config::Schema CacheComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<ComponentBase>(
      R"(
type: object
additionalProperties: false
description: Cache component
properties:
  max_size:
    type: integer
    minimum: 0
    description: Maximum amount of elements in cache
)");
}

std::shared_ptr<::cache::ICache> CacheComponent::GetCache(
    const std::string& key) {
  if (auto it = caches_.find(key); it != caches_.end()) {
    return it->second;
  }

  caches_.emplace(key, CreateCache());
  return caches_[key];
}

std::shared_ptr<::cache::ICache> CacheComponent::CreateCache() {
  return std::make_shared<::cache::InMemoryCache>(cache_max_size_);
}

}  // namespace components::cache