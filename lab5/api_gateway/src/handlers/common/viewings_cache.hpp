#pragma once
#include <boost/uuid/uuid.hpp>
#include <cache/interface.hpp>
#include <components/serializers.hpp>
#include <components/viewing_storage/viewing_storage.hpp>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <userver/formats/json.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/text.hpp>

namespace components::viewing_storage {

inline Viewing Parse(const userver::formats::json::Value& value,
                     userver::formats::parse::To<Viewing>) {
  return {
      .user_id = value["user_id"].As<boost::uuids::uuid>(),
      .property_id = value["property_id"].As<boost::uuids::uuid>(),
      .viewing_date =
          value["viewing_date"].As<userver::utils::datetime::Date>(),
  };
}

inline userver::formats::json::Value Serialize(
    const Viewing& value,
    userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["user_id"] = value.user_id;
  builder["property_id"] = value.property_id;
  builder["viewing_date"] = value.viewing_date;

  return builder.ExtractValue();
}

}  // namespace components::viewing_storage

namespace handlers::common::viewings_cache {

enum class FilterEntityType { User, Property };

struct SearchParams {
  FilterEntityType entity_type;
  boost::uuids::uuid entity_id;
};

using SearchResult = std::vector<
    std::pair<boost::uuids::uuid, components::viewing_storage::Viewing>>;

const std::unordered_map<FilterEntityType, std::string>
    kFilterEntityTypeValues = {
        {FilterEntityType::Property, "property"},
        {FilterEntityType::User, "user"},
};

class ViewingsCache final {
 public:
  ViewingsCache(std::shared_ptr<cache::ICache> cache) : cache_(cache) {}

  std::optional<SearchResult> Get(const SearchParams& params) {
    const auto key = SerializeParams(params);
    auto cached_value = cache_->get(key);

    std::cout << "[ViewingsCache] cached value for key '" << key << "'"
              << (cached_value.has_value() ? "found" : "not found")
              << std::endl;

    return cached_value.has_value()
               ? std::optional(DeserializeResult(cached_value.value()))
               : std::nullopt;
  }

  void Set(const SearchParams& params, const SearchResult& search_result,
           std::chrono::seconds ttl) {
    const auto key = SerializeParams(params);
    cache_->set(key, SerializeResult(search_result), ttl);

    std::cout << "[ViewingsCache] set cache value for key '" << key << "'"
              << std::endl;
  }

  void Invalidate(const SearchParams& params) {
    const auto key = SerializeParams(params);
    cache_->invalidate(key);

    std::cout << "[ViewingsCache] invalidate cache value for key '" << key
              << "'" << std::endl;
  }

 private:
  std::shared_ptr<cache::ICache> cache_;

  static cache::Key SerializeParams(const SearchParams& params) {
    return userver::utils::text::Join(
        {SerializeEntityType(params.entity_type),
         userver::utils::ToString(params.entity_id)},
        ":");
  }

  static cache::Value SerializeResult(const SearchResult& result) {
    userver::formats::json::ValueBuilder builder;
    builder["value"] = result;
    return userver::formats::json::ToString(builder.ExtractValue()["value"]);
  }

  static SearchResult DeserializeResult(const ::cache::Value& value) {
    const auto deserialized_value = userver::formats::json::FromString(value);
    return deserialized_value.As<SearchResult>();
  }

  static std::string SerializeEntityType(const FilterEntityType& entity_type) {
    return kFilterEntityTypeValues.at(entity_type);
  }
};

}  // namespace handlers::common::viewings_cache