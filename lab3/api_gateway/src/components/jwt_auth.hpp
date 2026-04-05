#pragma once

#include <chrono>
#include <stdexcept>
#include <string>
#include <userver/components/component_base.hpp>

namespace components::jwt_auth {

struct TokenPair {
  std::string access_token;
  std::string refresh_token;
};

class InvalidToken : std::runtime_error {
 public:
  InvalidToken(const std::string& message);
};

class JwtAuthComponent final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "jwt-auth";

  explicit JwtAuthComponent(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

  static userver::yaml_config::Schema GetStaticConfigSchema();

  TokenPair GenerateToken(const std::string& payload) const;
  std::string ValidateToken(const std::string& access_token) const;
  TokenPair RefreshToken(const std::string& refresh_token) const;

 private:
  std::string issuer_;
  std::string access_secret_;
  std::string refresh_secret_;
  std::chrono::seconds access_lifetime_;
  std::chrono::seconds refresh_lifetime_;
};
}  // namespace components::jwt_auth