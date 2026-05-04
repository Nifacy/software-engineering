#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace cache {

using Key = std::string;
using Value = std::string;

class ICache {
 public:
  virtual ~ICache() = default;
  virtual std::optional<Value> get(const Key& key) = 0;
  virtual void set(const Key& key, const Value& value,
                   std::chrono::seconds ttl) = 0;
  virtual void invalidate(const Key& key) = 0;
};

}  // namespace cache
