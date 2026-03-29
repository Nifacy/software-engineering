#pragma once

#include <components/property_storage.hpp>
#include <components/viewing_storage.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::update_property_handler {

class UpdatePropertyHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "update-property-handler";

  UpdatePropertyHandler(const userver::components::ComponentConfig& config,
                        const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::property_storage::PropertyStorage& property_storage_;
};

}  // namespace handlers::update_property_handler
