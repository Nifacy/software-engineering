#include <cache/interface.hpp>
#include <chrono>
#include <list>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace cache {

class InMemoryCache : public ICache {
 private:
  struct Entry {
    Value value;
    std::chrono::steady_clock::time_point expires_at;
    typename std::list<Key>::iterator lru_iter;
  };

  std::unordered_map<Key, Entry> store_;
  std::list<Key> lru_list_;
  size_t max_size_;
  std::mutex mtx_;

 public:
  explicit InMemoryCache(size_t max_size) : max_size_(max_size) {}

  std::optional<Value> get(const Key& key) override {
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

  void set(const Key& key, const Value& value,
           std::chrono::seconds ttl) override {
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

  void invalidate(const Key& key) override {
    std::lock_guard<std::mutex> lock(mtx_);
    if (auto it = store_.find(key); it != store_.end()) {
      lru_list_.erase(it->second.lru_iter);
      store_.erase(it);
    }
  }
};

}  // namespace cache
