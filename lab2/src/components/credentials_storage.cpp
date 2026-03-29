#include <components/credentials_storage.hpp>
#include <userver/components/component_context.hpp>

namespace components::credentials_storage {

CredentialsAlreadyExists::CredentialsAlreadyExists(const std::string& key)
    : std::runtime_error("Credentials by key '" + key + "' already exist") {}

CredentialsNotFound::CredentialsNotFound(const std::string& key)
    : std::runtime_error("Credentials by key '" + key + "' not found") {}

InvalidVerifySecret::InvalidVerifySecret(const std::string& key)
    : std::runtime_error(
          "Passed invalid verify secret for credentials by key '" + key + "'") {
}

CredentialsStorage::CredentialsStorage(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context), credentials_() {}

void CredentialsStorage::AddCredentials(const std::string& key,
                                        const Credentials& credentials) {
  if (credentials_.find(key) != credentials_.end()) {
    throw CredentialsAlreadyExists(key);
  }

  credentials_.emplace(key, std::move(credentials));
}

std::string CredentialsStorage::VerifyCredentials(
    const std::string& key, const std::string& verify_secret) const {
  auto it = credentials_.find(key);
  if (it == credentials_.end()) {
    throw CredentialsNotFound(key);
  }

  auto credentials = it->second;
  if (credentials.verify_secret != verify_secret) {
    throw InvalidVerifySecret(key);
  }

  return credentials.payload;
}

}  // namespace components::credentials_storage