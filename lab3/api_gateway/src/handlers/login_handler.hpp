#pragma once

#include <components/credentials_storage/credentials_storage.hpp>
#include <components/jwt_auth/jwt_auth.hpp>
#include <handlers/common/schema_http_handler.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::login_handler {

class LoginHandler final : public common::SchemaHttpHandler {
 public:
  static constexpr std::string_view kName = "login-handler";

  LoginHandler(const userver::components::ComponentConfig& config,
               const userver::components::ComponentContext& context);

 protected:
  common::Response HandleRequestImpl(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::credentials_storage::CredentialsStorage& credentials_storage_;
  components::jwt_auth::JwtAuthComponent& jwt_auth_;
};

}  // namespace handlers::login_handler