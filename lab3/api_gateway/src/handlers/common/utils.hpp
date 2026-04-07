#pragma once

#include <boost/uuid/uuid.hpp>
#include <components/property_storage/property_storage.hpp>
#include <schemas/common_fwd.hpp>
#include <schemas/property_fwd.hpp>
#include <userver/server/http/http_request.hpp>

namespace handlers::common {

api_gateway::schemas::property::PropertyStatus SerializePropertyStatus(
    const components::property_storage::PropertyStatus&);

api_gateway::schemas::common::Address SerializeAddress(
    const components::property_storage::Address&);

std::vector<std::string> ConvertUuidToString(
    const std::vector<boost::uuids::uuid>& ids);

std::optional<boost::uuids::uuid> TryGetUuidPathArgs(
    const userver::server::http::HttpRequest& request, const std::string& key);

}  // namespace handlers::common