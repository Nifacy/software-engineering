#include <handlers/common/utils.hpp>
#include <schemas/common.hpp>
#include <schemas/property.hpp>
#include <userver/utils/boost_uuid4.hpp>

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
  // TODO: remove this
  const auto apartment = address.apartment.has_value() ? *address.apartment : 0;

  return {
      .country = address.country,
      .city = address.city,
      .street = address.street,
      .building = (std::int32_t)address.building,
      .apartment = (std::int32_t)apartment,
  };
}

std::vector<std::string> ConvertUuidToString(
    const std::vector<boost::uuids::uuid>& ids) {
  std::vector<std::string> result;

  for (const auto& el : ids) {
    result.push_back(userver::utils::ToString(el));
  }

  return result;
}

std::optional<boost::uuids::uuid> TryGetUuidPathArgs(
    const userver::server::http::HttpRequest& request,
    const std::string& arg_name) {
  const auto arg_value = request.GetPathArg(arg_name);

  try {
    return userver::utils::BoostUuidFromString(arg_value);
  } catch (std::exception&) {
    return std::nullopt;
  }
}

}  // namespace handlers::common