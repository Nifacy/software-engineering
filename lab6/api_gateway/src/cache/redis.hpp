#include <cache/interface.hpp>
#include <userver/storages/redis/client.hpp>

namespace cache {

class RedisCache : public ICache {
 private:
  userver::storages::redis::ClientPtr redis_client_;
  userver::storages::redis::CommandControl cmd_control_;
  const std::string key_prefix_;

  std::string MakeKey(const Key& key) const;

 public:
  RedisCache(const userver::storages::redis::ClientPtr& redis_client,
             const std::string& key_prefix,
             const std::chrono::seconds& timeout);

  std::optional<Value> get(const Key& key) override;
  void invalidate(const Key& key) override;
  std::vector<Key> keys() override;
  void set(const Key& key, const Value& value,
           std::chrono::seconds ttl) override;
};

}  // namespace cache
