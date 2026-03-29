#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "components/user_storage.hpp"

namespace handlers::find_users_handler {

class FindUsersHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "find-users-handler";

  FindUsersHandler(const userver::components::ComponentConfig& config,
                   const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::user_storage::UserStorage& user_storage_;
};

}  // namespace handlers::find_users_handler