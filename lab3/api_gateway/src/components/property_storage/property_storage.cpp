#include <components/property_storage/property_storage.hpp>
#include <queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/io_fwd.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/trivial_map.hpp>

namespace components::property_storage {
enum class PostgresPropertyStatus { kActive, kSold };

struct PostgresProperty {
  boost::uuids::uuid owner_id;
  boost::uuids::uuid address_id;
  PostgresPropertyStatus status;
  int price;
};

struct PostgresPropertyWithAddress {
  boost::uuids::uuid owner_id;
  PostgresPropertyStatus status;
  int price;
  std::string address__country;
  std::string address__city;
  std::string address__street;
  int address__building;
  std::optional<int> address__apartment;
};
}  // namespace components::property_storage

namespace userver::storages::postgres::io {

using PostgresPropertyStatus =
    ::components::property_storage::PostgresPropertyStatus;

template <>
struct CppToUserPg<PostgresPropertyStatus> {
  static constexpr DBTypeName postgres_name = "public.property_status";
  static constexpr USERVER_NAMESPACE::utils::TrivialBiMap enumerators =
      [](auto selector) {
        return selector()
            .Case("active", PostgresPropertyStatus::kActive)
            .Case("sold", PostgresPropertyStatus::kSold);
      };
};
}  // namespace userver::storages::postgres::io

namespace components::property_storage {

static std::unordered_map<PropertyStatus, PostgresPropertyStatus>
    kStatusValues = {
        {PropertyStatus::Active, PostgresPropertyStatus::kActive},
        {PropertyStatus::Sold, PostgresPropertyStatus::kSold},
};

boost::uuids::uuid createAdddress(
    userver::storages::postgres::Transaction& transaction,
    const Address& address) {
  const auto result = transaction.Execute(
      queries::sql::kCreateAddress, address.country, address.city,
      address.street, address.building, address.apartment);

  return result.AsSingleRow<boost::uuids::uuid>();
}

boost::uuids::uuid createProperty(
    userver::storages::postgres::Transaction& transaction,
    const PostgresProperty& property) {
  const auto result =
      transaction.Execute(queries::sql::kCreateProperty, property.owner_id,
                          property.address_id, property.status, property.price);

  return result.AsSingleRow<boost::uuids::uuid>();
}

std::optional<PostgresPropertyWithAddress> tryGetPropertyById(
    userver::storages::postgres::ClusterPtr cluster,
    const boost::uuids::uuid& property_id) {
  const auto result =
      cluster->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                       queries::sql::kGetPropertyById, property_id);

  return result.IsEmpty()
             ? std::nullopt
             : std::optional(result.AsSingleRow<PostgresPropertyWithAddress>(
                   userver::storages::postgres::kRowTag));
}

bool tryUpdateProperty(userver::storages::postgres::Transaction& transaction,
                       const boost::uuids::uuid& property_id,
                       const std::optional<PropertyStatus>& status,
                       const std::optional<int> price) {
  const auto serialized_status = status.has_value()
                                     ? std::optional(kStatusValues.at(*status))
                                     : std::nullopt;

  const auto result = transaction.Execute(
      queries::sql::kUpdateProperty, property_id, serialized_status, price);

  return !result.IsEmpty();
}

PropertyStatus parsePropertyStatus(const PostgresPropertyStatus& value) {
  for (const auto& item : kStatusValues) {
    if (item.second == value) {
      return item.first;
    }
  }
  throw std::invalid_argument("Unknown property status");
}

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
      cluster_(context.FindComponent<userver::components::Postgres>("database")
                   .GetCluster()) {}

boost::uuids::uuid PropertyStorage::CreateProperty(const Property& property) {
  auto transaction = cluster_->Begin(
      "create_property", userver::storages::postgres::ClusterHostType::kMaster,
      userver::storages::postgres::Transaction::RW);

  const auto address_id = createAdddress(transaction, property.address);
  const auto property_id = createProperty(
      transaction, {
                       .owner_id = property.owner_id,
                       .address_id = address_id,
                       .status = kStatusValues.at(property.status),
                       .price = property.price,
                   });

  transaction.Commit();

  return property_id;
}

void PropertyStorage::UpdateProperty(
    const boost::uuids::uuid& property_id,
    const std::optional<PropertyStatus>& status,
    const std::optional<int>& price) {
  auto transaction = cluster_->Begin(
      "update_property", userver::storages::postgres::ClusterHostType::kMaster,
      userver::storages::postgres::Transaction::RW);

  const auto updated =
      tryUpdateProperty(transaction, property_id, status, price);

  if (!updated) {
    throw PropertyNotFound(property_id);
  }

  transaction.Commit();
}

Property PropertyStorage::GetProperty(
    const boost::uuids::uuid& property_id) const {
  const auto maybe_result = tryGetPropertyById(cluster_, property_id);
  if (!maybe_result.has_value()) {
    throw PropertyNotFound(property_id);
  }

  const auto result = *maybe_result;
  return {
      .owner_id = result.owner_id,
      .address =
          {
              .country = result.address__country,
              .city = result.address__city,
              .street = result.address__street,
              .building = result.address__building,
              .apartment = result.address__apartment,
          },
      .status = parsePropertyStatus(result.status),
      .price = result.price,
  };
}

std::vector<boost::uuids::uuid> PropertyStorage::FindProperties(
    const std::optional<std::string>& city_pattern,
    const std::optional<int>& min_price, const std::optional<int>& max_price,
    const std::optional<boost::uuids::uuid>& owner_id) {
  const auto result =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                        queries::sql::kFindProperties, city_pattern, min_price,
                        max_price, owner_id);

  return result.AsContainer<std::vector<boost::uuids::uuid>>();
}

}  // namespace components::property_storage
