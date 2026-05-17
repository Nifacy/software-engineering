#include <auth/auth_checker.hpp>

#include <userver/components/component.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/logging/log.hpp>
#include <userver/utils/boost_uuid4.hpp>

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
  try {
    LOG_INFO() << "Get authorization header ...";
    const std::string_view auth_header =
        request.GetHeader(userver::http::headers::kAuthorization);

    LOG_INFO() << "Check authorization header existence ...";
    if (auth_header.empty()) {
      return {
          .status = AuthCheckResult::Status::kTokenNotFound,
          .reason = "Missing 'Authorization' header",
      };
    }

    LOG_INFO() << "Check authorization header algorithm ...";
    if (!auth_header.starts_with(kAlgorithm)) {
      return {
          .status = AuthCheckResult::Status::kInvalidToken,
          .reason = "Invalid authorization type, expected 'Bearer'",
      };
    }

    LOG_INFO() << "Retrieve access token ...";
    const std::string access_token =
        std::string(auth_header.substr(kAlgorithm.length()));

    try {
      LOG_INFO() << "Validate token ...";
      const auto payload = jwt_auth_.ValidateToken(access_token);
      LOG_INFO() << "Parse token payload ...";
      const auto user_id = userver::utils::BoostUuidFromString(payload);
      LOG_INFO() << "Set parsed user ID to request context ...";
      request_context.SetData("user_id", user_id);
      return {};
    } catch (const components::jwt_auth::InvalidToken&) {
      return {
          .status = AuthCheckResult::Status::kInvalidToken,
          .reason = "Invalid token",
      };
    }
  } catch (const std::exception& error) {
    LOG_ERROR() << "Unexpected error while checking auth credentials: "
                << error.what();
    throw error;
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