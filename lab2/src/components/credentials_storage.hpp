#pragma once

#include <string>
#include <unordered_set>
#include <userver/components/component_base.hpp>

namespace components::credentials_storage {

class CredentialsStorage final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "credentials-storage";

  explicit CredentialsStorage(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

  void AddCredentials(const std::string& credentials);

  bool HasCredentials(const std::string& credentials) const;

 private:
  std::unordered_set<std::string> credentials_;
};

}  // namespace components::credentials_storage
