#pragma once

#include <components/property_storage.hpp>
#include <components/viewing_storage.hpp>
#include <handlers/common/schema_http_handler.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::delete_viewing_handler {

class DeleteViewingHandler final : public handlers::common::SchemaHttpHandler {
 public:
  static constexpr std::string_view kName = "delete-viewing-handler";

  DeleteViewingHandler(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);

 protected:
  handlers::common::Response HandleRequestImpl(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::viewing_storage::ViewingStorage& viewing_storage_;
  components::property_storage::PropertyStorage& property_storage_;
};

}  // namespace handlers::delete_viewing_handler
