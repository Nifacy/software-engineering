#include "credentials_storage.hpp"
#include <userver/components/component_context.hpp>

namespace components::credentials_storage {

CredentialsStorage::CredentialsStorage(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context) {}

void CredentialsStorage::AddCredentials(const std::string& credentials) {
  credentials_.insert(credentials);
}

bool CredentialsStorage::HasCredentials(const std::string& credentials) const {
  return credentials_.count(credentials) > 0;
}

}  // namespace components::credentials_storage