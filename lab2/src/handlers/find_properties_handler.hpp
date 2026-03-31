#pragma once

#include <handlers/common/schema_http_handler.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include "components/property_storage.hpp"

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
  std::optional<int> TryGetIntArg(const userver::server::http::HttpRequest&,
                                  const std::string&) const;

  components::property_storage::PropertyStorage& property_storage_;
};

}  // namespace handlers::find_properties_handler