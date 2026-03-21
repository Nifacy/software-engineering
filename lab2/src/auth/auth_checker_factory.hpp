#pragma once

#include <userver/server/handlers/auth/auth_checker_base.hpp>
#include <userver/server/handlers/auth/auth_checker_factory.hpp>

#include <auth/auth_checker.hpp>

namespace auth {

class AuthCheckerFactory final
    : public userver::server::handlers::auth::AuthCheckerFactoryBase {
 public:
  static constexpr const char* kAuthType = "simple-auth";

  AuthCheckerFactory(const userver::components::ComponentContext& context);

  userver::server::handlers::auth::AuthCheckerBasePtr MakeAuthChecker(
      const userver::server::handlers::auth::HandlerAuthConfig&) const override;

 private:
  const auth::AuthCheckerComponent& checker_component_;
};

}  // namespace auth