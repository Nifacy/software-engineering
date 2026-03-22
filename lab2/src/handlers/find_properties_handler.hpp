#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "components/property_storage.hpp"

namespace handlers::find_properties_handler {

class FindPropertiesHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "find-properties-handler";

  FindPropertiesHandler(const userver::components::ComponentConfig& config,
                        const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::property_storage::PropertyStorage& property_storage_;
};

}  // namespace handlers::find_properties_handler