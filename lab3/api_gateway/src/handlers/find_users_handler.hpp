#pragma once

#include <components/user_storage.hpp>
#include <handlers/common/schema_http_handler.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::find_users_handler {

class FindUsersHandler final : public common::SchemaHttpHandler {
 public:
  static constexpr std::string_view kName = "find-users-handler";

  FindUsersHandler(const userver::components::ComponentConfig& config,
                   const userver::components::ComponentContext& context);

 protected:
  common::Response HandleRequestImpl(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::user_storage::UserStorage& user_storage_;
};

}  // namespace handlers::find_users_handler