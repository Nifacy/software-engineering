#pragma once
#include <boost/uuid/uuid.hpp>
#include <cache/interface.hpp>
#include <memory>

namespace handlers::common::property_cache {

struct SearchParams {
  std::optional<std::string> city_pattern;
  std::optional<int> min_price;
  std::optional<int> max_price;
};

struct PropertyInfo {
  std::string city;
  int price;
};

using SearchResult = std::vector<boost::uuids::uuid>;
using CachePtr = std::shared_ptr<::cache::ICache>;

class PropertyCache final {
 public:
  PropertyCache(const CachePtr cache);
  std::optional<SearchResult> Get(const SearchParams& params);
  void Set(const SearchParams& params, const SearchResult& search_result,
           std::chrono::seconds ttl);
  void Invalidate(const PropertyInfo& info);

 private:
  CachePtr cache_;

  static ::cache::Key SerializeParams(const SearchParams& params);
  static SearchParams DeserializeParams(const ::cache::Key& key);
  static ::cache::Value SerializeResult(const SearchResult& result);
  static SearchResult DeserializeResult(const ::cache::Value& value);
  static bool MatchesParams(const PropertyInfo& info,
                            const SearchParams& params);
};

}  // namespace handlers::common::property_cache