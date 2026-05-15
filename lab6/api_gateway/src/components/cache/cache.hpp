#pragma once

#include <cache/interface.hpp>
#include <memory>
#include <string>
#include <userver/components/component_base.hpp>

namespace components::cache {

class CacheComponent final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "cache";

  explicit CacheComponent(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context);

  static userver::yaml_config::Schema GetStaticConfigSchema();
  std::shared_ptr<::cache::ICache> GetCache(const std::string& key);

 private:
  std::shared_ptr<::cache::ICache> CreateCache();
  std::unordered_map<std::string, std::shared_ptr<::cache::ICache>> caches_;
  size_t cache_max_size_;
};

}  // namespace components::cache