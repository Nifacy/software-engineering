#include <components/credentials_storage.hpp>
#include <userver/server/handlers/auth/auth_checker_base.hpp>

namespace auth {

class AuthChecker final
    : public userver::server::handlers::auth::AuthCheckerBase {
 public:
  using AuthCheckResult = userver::server::handlers::auth::AuthCheckResult;

  AuthChecker(const components::credentials_storage::CredentialsStorage&
                  credentials_storage);

  AuthCheckResult CheckAuth(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext& request_context) const override;

  bool SupportsUserAuth() const noexcept override { return true; }

 private:
  const components::credentials_storage::CredentialsStorage&
      credentials_storage_;
};

class AuthCheckerComponent final : public userver::components::ComponentBase {
 public:
  static constexpr auto kName = "auth-checker";

  AuthCheckerComponent(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);

  userver::server::handlers::auth::AuthCheckerBasePtr GetChecker() const;

 private:
  userver::server::handlers::auth::AuthCheckerBasePtr checker_;
};
}  // namespace auth