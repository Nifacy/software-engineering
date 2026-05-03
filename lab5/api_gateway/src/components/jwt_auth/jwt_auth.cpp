#include <jwt-cpp/jwt.h>

#include <components/jwt_auth/jwt_auth.hpp>
#include <userver/components/component_config.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace components::jwt_auth {

InvalidToken::InvalidToken(const std::string& message)
    : std::runtime_error("Received token is invalid: " + message) {}

std::string createToken(const std::string& payload,
                        const std::chrono::seconds& lifetime,
                        const std::string& secret, const std::string& issuer,
                        const std::string& token_type) {
  auto now = std::chrono::system_clock::now();

  return jwt::create()
      .set_issuer(issuer)
      .set_type("JWS")
      .set_payload_claim("payload", jwt::claim(payload))
      .set_payload_claim("token_type", jwt::claim(token_type))
      .set_issued_at(now)
      .set_expires_at(now + lifetime)
      .sign(jwt::algorithm::hs256{secret});
}

std::string validateToken(const std::string& token, const std::string& secret,
                          const std::string& issuer) {
  try {
    auto verifier = jwt::verify()
                        .allow_algorithm(jwt::algorithm::hs256{secret})
                        .with_issuer(issuer);

    auto decoded = jwt::decode(token);
    verifier.verify(decoded);

    auto payload_claim = decoded.get_payload_claim("payload");
    return payload_claim.as_string();
  } catch (const jwt::error::token_verification_exception& exc) {
    throw InvalidToken(exc.what());
  } catch (const jwt::error::signature_verification_exception& exc) {
    throw InvalidToken(exc.what());
  }
}

JwtAuthComponent::JwtAuthComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      issuer_(config["issuer"].As<std::string>()),
      access_secret_(config["access_secret"].As<std::string>()),
      refresh_secret_(config["refresh_secret"].As<std::string>()),
      access_lifetime_(config["access_lifetime"].As<long>()),
      refresh_lifetime_(config["refresh_lifetime"].As<long>()) {}

TokenPair JwtAuthComponent::GenerateToken(const std::string& payload) const {
  return {
      .access_token = createToken(payload, access_lifetime_, access_secret_,
                                  issuer_, "access"),
      .refresh_token = createToken(payload, refresh_lifetime_, refresh_secret_,
                                   issuer_, "refresh"),
  };
}

std::string JwtAuthComponent::ValidateToken(
    const std::string& access_token) const {
  return validateToken(access_token, access_secret_, issuer_);
}

TokenPair JwtAuthComponent::RefreshToken(
    const std::string& refresh_token) const {
  const auto payload = validateToken(refresh_token, refresh_secret_, issuer_);
  return GenerateToken(payload);
}

userver::yaml_config::Schema JwtAuthComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<ComponentBase>(
      R"(
type: object
additionalProperties: false
description: JWT auth checker component
properties:
  access_secret:
    type: string
    description: Access token secret
  refresh_secret:
    type: string
    description: Refresh token secret
  access_lifetime:
    type: integer
    description: Lifetime of access token (in seconds)
  refresh_lifetime:
    type: integer
    description: Lifetime of refresh token (in seconds)
  issuer:
    type: string
    description: Identifier of token issuer
)");
}

}  // namespace components::jwt_auth