#include <components/property_storage/property_storage.hpp>
#include <components/serializers.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/bson.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/trivial_map.hpp>

namespace userver::formats::serialize {

using Address = ::components::property_storage::Address;
using PropertyStatus = ::components::property_storage::PropertyStatus;

bson::Value Serialize(const Address& value, To<bson::Value>) {
  bson::ValueBuilder builder;

  builder["country"] = value.country;
  builder["city"] = value.city;
  builder["street"] = value.street;
  builder["building"] = value.building;

  if (value.apartment.has_value()) {
    builder["apartment"] = *value.apartment;
  }

  return builder.ExtractValue();
}

bson::Value Serialize(const PropertyStatus& status, To<bson::Value>) {
  switch (status) {
    case PropertyStatus::Active:
      return bson::MakeDoc("value", "active")["value"];
    case PropertyStatus::Sold:
      return bson::MakeDoc("value", "sold")["value"];
  }
}

}  // namespace userver::formats::serialize

namespace userver::formats::parse {

using Property = ::components::property_storage::Property;
using Address = ::components::property_storage::Address;
using PropertyStatus = ::components::property_storage::PropertyStatus;

Address Parse(const bson::Value& value, To<Address>) {
  const auto apartment = value.HasMember("apartment")
                             ? std::optional(value["apartment"].As<int>())
                             : std::nullopt;

  return Address{
      .country = value["country"].As<std::string>(),
      .city = value["city"].As<std::string>(),
      .street = value["street"].As<std::string>(),
      .building = value["building"].As<int>(),
      .apartment = apartment,
  };
}

PropertyStatus Parse(const bson::Value& value, To<PropertyStatus>) {
  const auto raw_value = value.As<std::string>();

  if (raw_value == "active") return PropertyStatus::Active;
  if (raw_value == "sold") return PropertyStatus::Sold;
  throw std::runtime_error("Unknown value for property status");
}

Property Parse(const bson::Value& value, To<Property>) {
  return Property{
      .owner_id = value["owner_id"].As<boost::uuids::uuid>(),
      .address = value["address"].As<Address>(),
      .status = value["status"].As<PropertyStatus>(),
      .price = value["price"].As<int>(),
  };
}

}  // namespace userver::formats::parse

namespace components::property_storage {

namespace bson = userver::formats::bson;

PropertyAlreadyExists::PropertyAlreadyExists(const std::string& property_id)
    : std::runtime_error("Property with ID '" + property_id +
                         "' already exists") {}

PropertyNotFound::PropertyNotFound(const boost::uuids::uuid& property_id)
    : std::runtime_error("Property with ID '" +
                         userver::utils::ToString(property_id) +
                         "' not found") {}

PropertyStorage::PropertyStorage(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-database")
                .GetPool()) {}

boost::uuids::uuid PropertyStorage::CreateProperty(const Property& property) {
  const auto property_id = userver::utils::generators::GenerateBoostUuid();
  const auto date_document = bson::MakeDoc(
      "_id", property_id, "owner_id", property.owner_id, "address",
      property.address, "status", property.status, "price", property.price);

  auto collection = pool_->GetCollection("properties");
  collection.InsertOne(date_document);
  return property_id;
}

void PropertyStorage::UpdateProperty(
    const boost::uuids::uuid& property_id,
    const std::optional<PropertyStatus>& status,
    const std::optional<int>& price) {
  bson::ValueBuilder builder;
  if (status.has_value()) builder["status"] = *status;
  if (price.has_value()) builder["price"] = *price;

  const auto updated_fields =
      builder.IsEmpty() ? bson::MakeDoc() : builder.ExtractValue();

  const auto selector = bson::MakeDoc("_id", property_id);
  auto collection = pool_->GetCollection("properties");
  const auto result =
      collection.UpdateOne(selector, bson::MakeDoc("$set", updated_fields));

  if (result.MatchedCount() == 0) {
    throw PropertyNotFound(property_id);
  }
}

Property PropertyStorage::GetProperty(
    const boost::uuids::uuid& property_id) const {
  const auto selector = bson::MakeDoc("_id", property_id);
  auto collection = pool_->GetCollection("properties");
  const auto result = collection.FindOne(selector);

  if (!result.has_value()) {
    throw PropertyNotFound(property_id);
  }

  return (*result).As<Property>();
}

std::vector<boost::uuids::uuid> PropertyStorage::FindProperties(
    const std::optional<std::string>& city_pattern,
    const std::optional<int>& min_price, const std::optional<int>& max_price,
    const std::optional<boost::uuids::uuid>& owner_id) {
  bson::ValueBuilder builder;

  if (city_pattern.has_value()) {
    builder["address.city"] = bson::MakeDoc("$regex", *city_pattern);
  }

  if (min_price.has_value() || max_price.has_value()) {
    bson::ValueBuilder price_builder;
    if (max_price.has_value()) price_builder["$lte"] = *max_price;
    if (min_price.has_value()) price_builder["$gte"] = *min_price;
    builder["price"] = price_builder.ExtractValue();
  }

  if (owner_id.has_value()) {
    builder["owner_id"] = *owner_id;
  }

  const auto filter =
      builder.IsEmpty() ? bson::MakeDoc() : builder.ExtractValue();
  auto collection = pool_->GetCollection("properties");
  auto result = collection.Find(filter);
  std::vector<boost::uuids::uuid> property_ids;

  for (const auto& el : result) {
    property_ids.push_back(el["_id"].As<boost::uuids::uuid>());
  }

  return property_ids;
}

}  // namespace components::property_storage
