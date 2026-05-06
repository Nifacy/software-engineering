#include <cache/redis.hpp>
#include <userver/utils/text.hpp>

namespace cache {

std::string RedisCache::MakeKey(const Key& key) const {
  return userver::utils::text::Join({key_prefix_, key}, ":");
}

RedisCache::RedisCache(const userver::storages::redis::ClientPtr& redis_client,
                       const std::string& key_prefix,
                       const std::chrono::seconds& timeout)
    : redis_client_(redis_client),
      cmd_control_(timeout, std::chrono::seconds{30}, 3),
      key_prefix_(key_prefix) {}

std::optional<Value> RedisCache::get(const Key& key) {
  const auto result = redis_client_->Get(MakeKey(key), cmd_control_).Get();
  return result;
}

void RedisCache::set(const Key& key, const Value& value,
                     std::chrono::seconds ttl) {
  auto request = redis_client_->Set(
      MakeKey(key), value,
      std::chrono::duration_cast<std::chrono::milliseconds>(ttl), cmd_control_);
  request.Get();
}

void RedisCache::invalidate(const Key& key) {
  auto request = redis_client_->Del(MakeKey(key), cmd_control_);
  request.Get();
}

std::vector<Key> RedisCache::keys() {
  std::vector<Key> result;
  const auto scan_prefix = key_prefix_ + ":";
  const auto pattern = scan_prefix + "*";

  auto scan = redis_client_->Scan(0, userver::storages::redis::Match{pattern},
                                  cmd_control_);

  for (const auto& redis_key : scan) {
    if (redis_key.starts_with(scan_prefix)) {
      result.push_back(redis_key.substr(scan_prefix.size()));
    }
  }
  return result;
}

}  // namespace cache
