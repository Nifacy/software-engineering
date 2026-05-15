#pragma once

#include <components/property_storage/property_storage.hpp>
#include <components/viewing_storage/viewing_storage.hpp>
#include <handlers/common/schema_http_handler.hpp>
#include <handlers/common/viewings_cache.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::get_property_viewings_handler {

using ViewingList = std::vector<
    std::pair<boost::uuids::uuid, components::viewing_storage::Viewing>>;

class GetPropertyViewingsHandler final : public common::SchemaHttpHandler {
 public:
  static constexpr std::string_view kName = "get-property-viewings-handler";

  GetPropertyViewingsHandler(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

 protected:
  common::Response HandleRequestImpl(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::viewing_storage::ViewingStorage& viewing_storage_;
  components::property_storage::PropertyStorage& property_storage_;
  mutable handlers::common::viewings_cache::ViewingsCache viewing_cache_;

  ViewingList FindViewings(const boost::uuids::uuid& property_id) const;
};

}  // namespace handlers::get_property_viewings_handler
