#include <cache/in_memory.hpp>

namespace cache {

InMemoryCache::InMemoryCache(size_t max_size) : max_size_(max_size) {}

std::optional<Value> InMemoryCache::get(const Key& key) {
  std::lock_guard<std::mutex> lock(mtx_);
  auto it = store_.find(key);
  if (it == store_.end()) return std::nullopt;

  if (std::chrono::steady_clock::now() >= it->second.expires_at) {
    lru_list_.erase(it->second.lru_iter);
    store_.erase(it);
    return std::nullopt;
  }

  lru_list_.splice(lru_list_.begin(), lru_list_, it->second.lru_iter);
  return it->second.value;
}

void InMemoryCache::set(const Key& key, const Value& value,
                        std::chrono::seconds ttl) {
  std::lock_guard<std::mutex> lock(mtx_);
  auto now = std::chrono::steady_clock::now();

  if (auto it = store_.find(key); it != store_.end()) {
    it->second.value = value;
    it->second.expires_at = now + ttl;
    lru_list_.splice(lru_list_.begin(), lru_list_, it->second.lru_iter);
    return;
  }

  if (store_.size() >= max_size_) {
    auto lru_key = lru_list_.back();
    lru_list_.pop_back();
    store_.erase(lru_key);
  }

  lru_list_.push_front(key);
  store_[key] = Entry{value, now + ttl, lru_list_.begin()};
}

void InMemoryCache::invalidate(const Key& key) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (auto it = store_.find(key); it != store_.end()) {
    lru_list_.erase(it->second.lru_iter);
    store_.erase(it);
  }
}

std::vector<Key> InMemoryCache::keys() {
  std::vector<Key> keys;
  keys.reserve(store_.size());

  for (const auto& [key, _] : store_) {
    keys.push_back(key);
  }

  return keys;
}

}  // namespace cache
