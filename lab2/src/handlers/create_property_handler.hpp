#pragma once

#include <components/property_storage.hpp>
#include <components/user_storage.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::create_property_handler {

class CreatePropertyHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "create-property-handler";

  CreatePropertyHandler(const userver::components::ComponentConfig& config,
                        const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::user_storage::UserStorage& user_storage_;
  components::property_storage::PropertyStorage& property_storage_;
};

}  // namespace handlers::create_property_handler