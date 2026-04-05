#include <boost/uuid/uuid.hpp>
#include <components/credentials_storage/credentials_storage.hpp>
#include <queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/boost_uuid4.hpp>

namespace components::credentials_storage {

struct credentialsRow {
  std::string verify_secret;
  boost::uuids::uuid user_id;
};

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
    : userver::components::ComponentBase(config, context),
      cluster_(context.FindComponent<userver::components::Postgres>("database")
                   .GetCluster()) {}

void CredentialsStorage::AddCredentials(const std::string& key,
                                        const Credentials& credentials) {
  try {
    auto transaction = cluster_->Begin(
        "create_user", userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Transaction::RW);

    const auto result = transaction.Execute(
        queries::sql::kCreateCredentials, key, credentials.verify_secret,
        userver::utils::BoostUuidFromString(credentials.payload));

    transaction.Commit();
  } catch (const userver::storages::postgres::UniqueViolation& e) {
    throw CredentialsAlreadyExists(key);
  }
}

std::string CredentialsStorage::VerifyCredentials(
    const std::string& key, const std::string& verify_secret) const {
  const auto result =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                        queries::sql::kGetCredentialsByKey, key);
  if (result.IsEmpty()) {
    throw CredentialsNotFound(key);
  }

  const auto credentials = result.AsSingleRow<credentialsRow>();
  if (credentials.verify_secret != verify_secret) {
    throw InvalidVerifySecret(key);
  }

  return userver::utils::ToString(credentials.user_id);
}

}  // namespace components::credentials_storage