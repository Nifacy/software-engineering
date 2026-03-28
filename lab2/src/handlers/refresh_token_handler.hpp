#pragma once

#include <components/jwt_auth.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::refresh_token_handler {

class RefreshTokenHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "refresh-token-handler";

  RefreshTokenHandler(const userver::components::ComponentConfig& config,
                      const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::jwt_auth::JwtAuthComponent& jwt_auth_;
};

}  // namespace handlers::refresh_token_handler