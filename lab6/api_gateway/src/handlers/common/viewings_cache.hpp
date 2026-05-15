#pragma once
#include <boost/uuid/uuid.hpp>
#include <cache/interface.hpp>
#include <components/viewing_storage/viewing_storage.hpp>
#include <vector>

namespace handlers::common::viewings_cache {

enum class FilterEntityType { User, Property };

struct SearchParams {
  FilterEntityType entity_type;
  boost::uuids::uuid entity_id;
};

using SearchResult = std::vector<
    std::pair<boost::uuids::uuid, components::viewing_storage::Viewing>>;

using CachePtr = std::shared_ptr<cache::ICache>;

class ViewingsCache final {
 public:
  ViewingsCache(CachePtr cache);
  std::optional<SearchResult> Get(const SearchParams& params);
  void Set(const SearchParams& params, const SearchResult& search_result,
           std::chrono::seconds ttl);
  void Invalidate(const SearchParams& params);

 private:
  CachePtr cache_;

  static cache::Key SerializeParams(const SearchParams& params);
  static cache::Value SerializeResult(const SearchResult& result);
  static SearchResult DeserializeResult(const ::cache::Value& value);
  static std::string SerializeEntityType(const FilterEntityType& entity_type);
};

}  // namespace handlers::common::viewings_cache