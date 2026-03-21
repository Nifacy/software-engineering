#include <auth/auth_checker.hpp>

#include <userver/components/component.hpp>
#include <userver/http/common_headers.hpp>

namespace auth {

static constexpr std::string_view kAlgorithm = "Bearer ";

AuthChecker::AuthChecker(
    const components::credentials_storage::CredentialsStorage&
        credentials_storage)
    : credentials_storage_(credentials_storage) {}

AuthChecker::AuthCheckResult AuthChecker::CheckAuth(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const std::string_view auth_header =
      request.GetHeader(userver::http::headers::kAuthorization);

  if (auth_header.empty()) {
    return AuthCheckResult{AuthCheckResult::Status::kTokenNotFound,
                           "Missing 'Authorization' header"};
  }

  if (!auth_header.starts_with(kAlgorithm)) {
    return AuthCheckResult{AuthCheckResult::Status::kInvalidToken,
                           "Invalid authorization type, expected 'Bearer'"};
  }

  const std::string token =
      std::string(auth_header.substr(kAlgorithm.length()));

  // TODO: don't copy string here
  if (credentials_storage_.HasCredentials(token)) {
    request_context.SetData("credentials", token);
    return AuthChecker::AuthCheckResult{};
  }

  return AuthCheckResult{AuthCheckResult::Status::kInvalidToken,

                         "Invalid credentials"};
}

AuthCheckerComponent::AuthCheckerComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      checker_(std::make_shared<AuthChecker>(
          context.FindComponent<
              components::credentials_storage::CredentialsStorage>())) {}

userver::server::handlers::auth::AuthCheckerBasePtr
AuthCheckerComponent::GetChecker() const {
  return checker_;
};

}  // namespace auth