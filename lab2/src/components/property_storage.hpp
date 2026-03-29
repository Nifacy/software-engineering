#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <userver/components/component_base.hpp>

namespace components::property_storage {

struct Address {
  std::string country;
  std::string city;
  std::string street;
  unsigned int building;
  unsigned int apartment;
};

enum class PropertyStatus { Active, Sold };

struct Property {
  std::string owner_id;
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
  explicit PropertyNotFound(const std::string& property_id);
};

class PropertyStorage final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "property-storage";

  explicit PropertyStorage(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

  void CreateProperty(const std::string& property_id, const Property& property);

  void UpdateProperty(const std::string& property_id, const Property& property);

  Property GetProperty(const std::string& property_id) const;

  std::vector<std::string> FindProperties(
      const std::optional<std::string>& city_pattern,
      const std::optional<int>& min_price, const std::optional<int>& max_price,
      const std::optional<std::string>& owner_id);

 private:
  std::unordered_map<std::string, Property> storage_;
};

}  // namespace components::property_storage
