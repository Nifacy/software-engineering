#pragma once

#include <boost/uuid/uuid.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <userver/components/component_base.hpp>
#include <userver/storages/mongo/pool.hpp>

namespace components::property_storage {

struct Address {
  std::string country;
  std::string city;
  std::string street;
  int building;
  std::optional<int> apartment;
};

enum class PropertyStatus { Active, Sold };

struct Property {
  boost::uuids::uuid owner_id;
  Address address;
  PropertyStatus status;
  int price;
};

class PropertyAlreadyExists : public std::runtime_error {
 public:
  explicit PropertyAlreadyExists(const std::string& property_id);
};

class PropertyNotFound : public std::runtime_error {
 public:
  explicit PropertyNotFound(const boost::uuids::uuid& property_id);
};

class PropertyStorage final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "property-storage";

  explicit PropertyStorage(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

  boost::uuids::uuid CreateProperty(const Property& property);

  void UpdateProperty(const boost::uuids::uuid& property_id,
                      const std::optional<PropertyStatus>& status,
                      const std::optional<int>& price);

  Property GetProperty(const boost::uuids::uuid& property_id) const;

  std::vector<boost::uuids::uuid> FindProperties(
      const std::optional<std::string>& city_pattern,
      const std::optional<int>& min_price, const std::optional<int>& max_price,
      const std::optional<boost::uuids::uuid>& owner_id);

 private:
  userver::storages::mongo::PoolPtr pool_;
};

}  // namespace components::property_storage
