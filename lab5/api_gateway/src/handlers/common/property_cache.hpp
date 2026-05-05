#pragma once
#include <boost/uuid/uuid.hpp>
#include <cache/interface.hpp>
#include <components/serializers.hpp>
#include <iostream>
#include <memory>
#include <userver/formats/json.hpp>
#include <userver/formats/parse/common.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/utils/from_string.hpp>
#include <userver/utils/text.hpp>

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

// TODO: Add normal logging
class PropertyCache final {
 public:
  PropertyCache(const std::shared_ptr<::cache::ICache> cache) : cache_(cache) {
    std::cout << "[PropertyCache] Created" << std::endl;
  }

  std::optional<SearchResult> Get(const SearchParams& params) {
    const auto key = SerializeParams(params);
    auto cached_value = cache_->get(key);

    const auto debug_value =
        cached_value.has_value() ? cached_value.value() : "NULL";
    std::cout << "[PropertyCache] Get entry by key '" << key
              << "': " << debug_value << std::endl;

    return cached_value.has_value()
               ? std::optional(DeserializeResult(cached_value.value()))
               : std::nullopt;
  }

  void Set(const SearchParams& params, const SearchResult& search_result,
           std::chrono::seconds ttl) {
    const auto key = SerializeParams(params);
    std::cout << "[PropertyCache] Set entry by key '" << key << "'"
              << std::endl;
    cache_->set(key, SerializeResult(search_result), ttl);
  }

  void Invalidate(const PropertyInfo& info) {
    std::vector<std::string> invalidate_keys;

    for (const auto& key : cache_->keys()) {
      const auto params = DeserializeParams(key);
      if (MatchesParams(info, params)) {
        invalidate_keys.push_back(key);
      }
    }

    for (const auto& key : invalidate_keys) {
      std::cout << "[PropertyCache] Invalidate entry by key '" << key << "'"
                << std::endl;
      cache_->invalidate(key);
    }
  }

 private:
  std::shared_ptr<::cache::ICache> cache_;

  static ::cache::Key SerializeParams(const SearchParams& params) {
    const auto min_price = params.min_price.has_value()
                               ? std::to_string(params.min_price.value())
                               : "*";
    const auto max_price = params.max_price.has_value()
                               ? std::to_string(params.max_price.value())
                               : "*";
    const auto city_pattern = params.city_pattern.value_or("*");

    return userver::utils::text::Join({city_pattern, min_price, max_price},
                                      ":");
  }

  static SearchParams DeserializeParams(const ::cache::Key& key) {
    SearchParams params;
    auto tokens = userver::utils::text::Split(key, ":");

    if (tokens.size() != 3) {
      throw std::invalid_argument("Invalid cache key format: " + key);
    }

    return {.city_pattern =
                tokens[0] == "*" ? std::nullopt : std::optional(tokens[0]),
            .min_price =
                tokens[1] == "*"
                    ? std::nullopt
                    : std::optional(userver::utils::FromString<int>(tokens[1])),
            .max_price = tokens[2] == "*"
                             ? std::nullopt
                             : std::optional(
                                   userver::utils::FromString<int>(tokens[2]))};
  }

  static ::cache::Value SerializeResult(const SearchResult& result) {
    userver::formats::json::ValueBuilder builder;
    builder["value"] = result;
    return userver::formats::json::ToString(builder.ExtractValue()["value"]);
  }

  static SearchResult DeserializeResult(const ::cache::Value& value) {
    const auto deserialized_value = userver::formats::json::FromString(value);
    return deserialized_value.As<SearchResult>();
  }

  static bool MatchesParams(const PropertyInfo& info,
                            const SearchParams& params) {
    const auto matches_city =
        !params.city_pattern.has_value() ||
        info.city.find(params.city_pattern.value()) != std::string::npos;

    const auto matches_min_price =
        !params.min_price.has_value() || info.price >= params.min_price.value();

    const auto matches_max_price =
        !params.max_price.has_value() || info.price <= params.max_price.value();

    return matches_city && matches_min_price && matches_max_price;
  }
};

}  // namespace handlers::common::property_cache