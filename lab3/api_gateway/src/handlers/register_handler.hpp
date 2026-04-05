#pragma once

#include <components/credentials_storage/credentials_storage.hpp>
#include <components/jwt_auth/jwt_auth.hpp>
#include <components/user_storage/user_storage.hpp>
#include <handlers/common/schema_http_handler.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::register_handler {

class RegisterHandler final : public common::SchemaHttpHandler {
 public:
  static constexpr std::string_view kName = "register-handler";

  RegisterHandler(const userver::components::ComponentConfig& config,
                  const userver::components::ComponentContext& context);

 protected:
  common::Response HandleRequestImpl(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::credentials_storage::CredentialsStorage& credentials_storage_;
  components::user_storage::UserStorage& user_storage_;
  components::jwt_auth::JwtAuthComponent& jwt_auth_;
};

}  // namespace handlers::register_handler