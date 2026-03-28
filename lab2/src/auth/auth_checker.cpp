#include <auth/auth_checker.hpp>

#include <userver/components/component.hpp>
#include <userver/http/common_headers.hpp>

namespace auth {

static constexpr std::string_view kAlgorithm = "Bearer ";

AuthChecker::AuthChecker(
    const components::credentials_storage::CredentialsStorage&
        credentials_storage,
    const components::jwt_auth::JwtAuthComponent& jwt_auth)
    : credentials_storage_(credentials_storage), jwt_auth_(jwt_auth) {}

AuthChecker::AuthCheckResult AuthChecker::CheckAuth(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& request_context) const {
  const std::string_view auth_header =
      request.GetHeader(userver::http::headers::kAuthorization);

  if (auth_header.empty()) {
    return {
        .status = AuthCheckResult::Status::kTokenNotFound,
        .reason = "Missing 'Authorization' header",
    };
  }

  if (!auth_header.starts_with(kAlgorithm)) {
    return {
        .status = AuthCheckResult::Status::kInvalidToken,
        .reason = "Invalid authorization type, expected 'Bearer'",
    };
  }

  const std::string access_token =
      std::string(auth_header.substr(kAlgorithm.length()));

  try {
    request_context.SetData("user_id", jwt_auth_.ValidateToken(access_token));
    return {};
  } catch (const components::jwt_auth::InvalidToken&) {
    return {
        .status = AuthCheckResult::Status::kInvalidToken,
        .reason = "Invalid token",
    };
  }
}

AuthCheckerComponent::AuthCheckerComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      checker_(std::make_shared<AuthChecker>(
          context.FindComponent<
              components::credentials_storage::CredentialsStorage>(),
          context.FindComponent<components::jwt_auth::JwtAuthComponent>())) {}

userver::server::handlers::auth::AuthCheckerBasePtr
AuthCheckerComponent::GetChecker() const {
  return checker_;
};

}  // namespace auth