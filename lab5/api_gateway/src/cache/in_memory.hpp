#include <cache/interface.hpp>
#include <list>
#include <mutex>
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
  explicit InMemoryCache(size_t max_size);

  std::optional<Value> get(const Key& key) override;
  void set(const Key& key, const Value& value,
           std::chrono::seconds ttl) override;
  void invalidate(const Key& key) override;
  std::vector<Key> keys() override;
};

}  // namespace cache
