#pragma once

#include <components/user_storage.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::get_user_handler {

class GetUserHandler final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "get-user-handler";

  GetUserHandler(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::user_storage::UserStorage& user_storage_;
};

}  // namespace handlers::get_user_handler