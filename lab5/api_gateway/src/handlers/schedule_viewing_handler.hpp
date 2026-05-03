#pragma once

#include <components/property_storage/property_storage.hpp>
#include <components/viewing_storage/viewing_storage.hpp>
#include <handlers/common/schema_http_handler.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::schedule_viewing_handler {

class ScheduleViewingHandler final : public common::SchemaHttpHandler {
 public:
  static constexpr std::string_view kName = "schedule-viewing-handler";

  ScheduleViewingHandler(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context);

 protected:
  common::Response HandleRequestImpl(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::viewing_storage::ViewingStorage& viewing_storage_;
  components::property_storage::PropertyStorage& property_storage_;
};

}  // namespace handlers::schedule_viewing_handler
