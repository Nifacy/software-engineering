#pragma once

#include <cache/interface.hpp>
#include <components/property_storage/property_storage.hpp>
#include <handlers/common/property_cache.hpp>
#include <handlers/common/schema_http_handler.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::find_properties_handler {

class FindPropertiesHandler final : public handlers::common::SchemaHttpHandler {
 public:
  static constexpr std::string_view kName = "find-properties-handler";

  FindPropertiesHandler(const userver::components::ComponentConfig& config,
                        const userver::components::ComponentContext& context);

 protected:
  common::Response HandleRequestImpl(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const final override;

 private:
  struct FindParams;

  std::optional<int> TryGetIntArg(const userver::server::http::HttpRequest&,
                                  const std::string&,
                                  const std::optional<int> minimum) const;

  std::vector<boost::uuids::uuid> FindProperties(
      const FindParams& params) const;

  components::property_storage::PropertyStorage& property_storage_;
  mutable handlers::common::property_cache::PropertyCache property_cache_;
};

}  // namespace handlers::find_properties_handler