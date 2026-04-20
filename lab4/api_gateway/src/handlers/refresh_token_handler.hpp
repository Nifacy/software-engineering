#pragma once

#include <components/jwt_auth/jwt_auth.hpp>
#include <handlers/common/schema_http_handler.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::refresh_token_handler {

class RefreshTokenHandler final : public common::SchemaHttpHandler {
 public:
  static constexpr std::string_view kName = "refresh-token-handler";

  RefreshTokenHandler(const userver::components::ComponentConfig& config,
                      const userver::components::ComponentContext& context);

 protected:
  common::Response HandleRequestImpl(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::jwt_auth::JwtAuthComponent& jwt_auth_;
};

}  // namespace handlers::refresh_token_handler