#pragma once

#include <components/viewing_storage.hpp>
#include <handlers/common/schema_http_handler.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::get_user_viewings_handler {

class GetUserViewingsHandler final : public common::SchemaHttpHandler {
 public:
  static constexpr std::string_view kName = "get-user-viewings-handler";

  GetUserViewingsHandler(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context);

 protected:
  common::Response HandleRequestImpl(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::viewing_storage::ViewingStorage& viewing_storage_;
};

}  // namespace handlers::get_user_viewings_handler