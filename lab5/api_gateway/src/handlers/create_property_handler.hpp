#pragma once

#include <components/property_storage/property_storage.hpp>
#include <components/user_storage/user_storage.hpp>
#include <handlers/common/property_cache.hpp>
#include <handlers/common/schema_http_handler.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::create_property_handler {

class CreatePropertyHandler final : public handlers::common::SchemaHttpHandler {
 public:
  static constexpr std::string_view kName = "create-property-handler";

  CreatePropertyHandler(const userver::components::ComponentConfig& config,
                        const userver::components::ComponentContext& context);

 protected:
  handlers::common::Response HandleRequestImpl(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::user_storage::UserStorage& user_storage_;
  components::property_storage::PropertyStorage& property_storage_;

  // TODO: add non-const 'HandleRequestImpl' support
  mutable handlers::common::property_cache::PropertyCache property_cache_;
};

}  // namespace handlers::create_property_handler