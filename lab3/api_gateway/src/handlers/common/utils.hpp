#pragma once

#include <components/property_storage/property_storage.hpp>
#include <schemas/common_fwd.hpp>
#include <schemas/property_fwd.hpp>

namespace handlers::common {

api_gateway::schemas::property::PropertyStatus SerializePropertyStatus(
    const components::property_storage::PropertyStatus&);

api_gateway::schemas::common::Address SerializeAddress(
    const components::property_storage::Address&);

}  // namespace handlers::common