#pragma once

#include <stdexcept>
#include <string>
#include <userver/components/component_base.hpp>

namespace components::credentials_storage {

struct Credentials {
  std::string verify_secret;
  std::string payload;
};

class CredentialsAlreadyExists : std::runtime_error {
 public:
  CredentialsAlreadyExists(const std::string& key);
};

class CredentialsNotFound : std::runtime_error {
 public:
  CredentialsNotFound(const std::string& key);
};

class InvalidVerifySecret : std::runtime_error {
 public:
  InvalidVerifySecret(const std::string& key);
};

class CredentialsStorage final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "credentials-storage";

  explicit CredentialsStorage(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

  void AddCredentials(const std::string& key, const Credentials& credentials);

  std::string VerifyCredentials(const std::string& key,
                                const std::string& verify_secret) const;

 private:
  std::unordered_map<std::string, Credentials> credentials_;
};

}  // namespace components::credentials_storage
