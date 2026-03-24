#pragma once

#include <components/property_storage.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::get_user_properties_handler {

class GetUserPropertiesHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "get-user-properties-handler";

  GetUserPropertiesHandler(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::property_storage::PropertyStorage& property_storage_;
};

}  // namespace handlers::get_user_properties_handler