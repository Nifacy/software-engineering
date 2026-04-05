#include <auth/auth_checker_factory.hpp>
#include <userver/components/component.hpp>

namespace auth {

AuthCheckerFactory::AuthCheckerFactory(
    const userver::components::ComponentContext& context)
    : AuthCheckerFactoryBase(),
      checker_component_(context.FindComponent<auth::AuthCheckerComponent>()) {}

userver::server::handlers::auth::AuthCheckerBasePtr
AuthCheckerFactory::MakeAuthChecker(
    const userver::server::handlers::auth::HandlerAuthConfig&) const {
  return checker_component_.GetChecker();
}

}  // namespace auth