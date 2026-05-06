#include <boost/uuid/uuid.hpp>
#include <components/serializers.hpp>
#include <handlers/common/property_cache.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/parse/common.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/utils/from_string.hpp>
#include <userver/utils/text.hpp>

namespace handlers::common::property_cache {

PropertyCache::PropertyCache(const CachePtr cache) : cache_(cache) {}

std::optional<SearchResult> PropertyCache::Get(const SearchParams& params) {
  const auto key = SerializeParams(params);
  auto cached_value = cache_->get(key);
  return cached_value.has_value()
             ? std::optional(DeserializeResult(cached_value.value()))
             : std::nullopt;
}

void PropertyCache::Set(const SearchParams& params,
                        const SearchResult& search_result,
                        std::chrono::seconds ttl) {
  const auto key = SerializeParams(params);
  cache_->set(key, SerializeResult(search_result), ttl);
}

void PropertyCache::Invalidate(const PropertyInfo& info) {
  std::vector<std::string> invalidate_keys;

  for (const auto& key : cache_->keys()) {
    const auto params = DeserializeParams(key);
    if (MatchesParams(info, params)) {
      invalidate_keys.push_back(key);
    }
  }

  for (const auto& key : invalidate_keys) {
    cache_->invalidate(key);
  }
}

::cache::Key PropertyCache::SerializeParams(const SearchParams& params) {
  const auto min_price = params.min_price.has_value()
                             ? std::to_string(params.min_price.value())
                             : "*";
  const auto max_price = params.max_price.has_value()
                             ? std::to_string(params.max_price.value())
                             : "*";
  const auto city_pattern = params.city_pattern.value_or("*");

  return userver::utils::text::Join({city_pattern, min_price, max_price}, ":");
}

SearchParams PropertyCache::DeserializeParams(const ::cache::Key& key) {
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
          .max_price =
              tokens[2] == "*"
                  ? std::nullopt
                  : std::optional(userver::utils::FromString<int>(tokens[2]))};
}

::cache::Value PropertyCache::SerializeResult(const SearchResult& result) {
  userver::formats::json::ValueBuilder builder;
  builder["value"] = result;
  return userver::formats::json::ToString(builder.ExtractValue()["value"]);
}

SearchResult PropertyCache::DeserializeResult(const ::cache::Value& value) {
  const auto deserialized_value = userver::formats::json::FromString(value);
  return deserialized_value.As<SearchResult>();
}

bool PropertyCache::MatchesParams(const PropertyInfo& info,
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

}  // namespace handlers::common::property_cache
