#include <boost/uuid/uuid.hpp>
#include <components/serializers.hpp>
#include <components/viewing_storage/viewing_storage.hpp>
#include <handlers/common/viewings_cache.hpp>
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

const std::unordered_map<FilterEntityType, std::string>
    kFilterEntityTypeValues = {
        {FilterEntityType::Property, "property"},
        {FilterEntityType::User, "user"},
};

ViewingsCache::ViewingsCache(std::shared_ptr<cache::ICache> cache)
    : cache_(cache) {}

std::optional<SearchResult> ViewingsCache::Get(const SearchParams& params) {
  const auto key = SerializeParams(params);
  auto cached_value = cache_->get(key);
  return cached_value.has_value()
             ? std::optional(DeserializeResult(cached_value.value()))
             : std::nullopt;
}

void ViewingsCache::Set(const SearchParams& params,
                        const SearchResult& search_result,
                        std::chrono::seconds ttl) {
  const auto key = SerializeParams(params);
  cache_->set(key, SerializeResult(search_result), ttl);
}

void ViewingsCache::Invalidate(const SearchParams& params) {
  const auto key = SerializeParams(params);
  cache_->invalidate(key);
}

cache::Key ViewingsCache::SerializeParams(const SearchParams& params) {
  return userver::utils::text::Join(
      {SerializeEntityType(params.entity_type),
       userver::utils::ToString(params.entity_id)},
      ":");
}

cache::Value ViewingsCache::SerializeResult(const SearchResult& result) {
  userver::formats::json::ValueBuilder builder;
  builder["value"] = result;
  return userver::formats::json::ToString(builder.ExtractValue()["value"]);
}

SearchResult ViewingsCache::DeserializeResult(const ::cache::Value& value) {
  const auto deserialized_value = userver::formats::json::FromString(value);
  return deserialized_value.As<SearchResult>();
}

std::string ViewingsCache::SerializeEntityType(
    const FilterEntityType& entity_type) {
  return kFilterEntityTypeValues.at(entity_type);
}

}  // namespace handlers::common::viewings_cache
