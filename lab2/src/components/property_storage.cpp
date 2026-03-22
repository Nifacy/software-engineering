#include <components/property_storage.hpp>

namespace components::property_storage {

PropertyAlreadyExists::PropertyAlreadyExists(const std::string& property_id)
    : std::runtime_error("Property with ID '" + property_id +
                         "' already exists") {}

PropertyNotFound::PropertyNotFound(const std::string& property_id)
    : std::runtime_error("Property with ID '" + property_id + "' not found") {}

PropertyStorage::PropertyStorage(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context), storage_() {}

void PropertyStorage::CreateProperty(const std::string& property_id,
                                     const Property& property) {
  if (storage_.find(property_id) != storage_.end()) {
    throw PropertyAlreadyExists(property_id);
  }

  storage_.emplace(property_id, std::move(property));
}

void PropertyStorage::UpdateProperty(const std::string& property_id,
                                     const Property& property) {
  auto it = storage_.find(property_id);
  if (it == storage_.end()) {
    throw PropertyNotFound(property_id);
  }
  it->second = property;
}

Property PropertyStorage::GetProperty(const std::string& property_id) const {
  auto it = storage_.find(property_id);
  if (it == storage_.end()) {
    throw PropertyNotFound(property_id);
  }

  return it->second;
}

std::vector<std::string> PropertyStorage::FindProperties(
    const std::optional<std::string>& city_pattern,
    const std::optional<int>& min_price, const std::optional<int>& max_price) {
  std::vector<std::string> result;

  for (const auto& [id, property] : storage_) {
    bool match_city =
        !city_pattern.has_value() ||
        (property.address.city.find(*city_pattern) != std::string::npos);

    bool match_min_price =
        !min_price.has_value() || (property.price >= *min_price);

    bool match_max_price =
        !max_price.has_value() || (property.price <= *max_price);

    if (match_city && match_min_price && match_max_price) {
      result.push_back(id);
    }
  }

  return result;
}

}  // namespace components::property_storage
