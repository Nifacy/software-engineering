#include <handlers/common/utils.hpp>
#include <schemas/common.hpp>
#include <schemas/property.hpp>

namespace handlers::common {

api_gateway::schemas::property::PropertyStatus SerializePropertyStatus(
    const components::property_storage::PropertyStatus& status) {
  switch (status) {
    case components::property_storage::PropertyStatus::Active:
      return api_gateway::schemas::property::PropertyStatus::kActive;
    case components::property_storage::PropertyStatus::Sold:
      return api_gateway::schemas::property::PropertyStatus::kSold;
  }
  throw std::runtime_error("Unknown property status");
}

api_gateway::schemas::common::Address SerializeAddress(
    const components::property_storage::Address& address) {
  return {
      .country = address.country,
      .city = address.city,
      .street = address.street,
      .building = (std::int32_t)address.building,
      .apartment = (std::int32_t)address.apartment,
  };
}

}  // namespace handlers::common