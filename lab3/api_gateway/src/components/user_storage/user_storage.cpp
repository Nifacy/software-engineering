#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <components/user_storage/user_storage.hpp>
#include <queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/boost_uuid4.hpp>

namespace components::user_storage {

UserAlreadyExistsException::UserAlreadyExistsException(const std::string& login)
    : std::runtime_error("User with login '" + login + "' already exists") {}

UserNotFound::UserNotFound(const boost::uuids::uuid& user_id)
    : std::runtime_error("User with ID '" + userver::utils::ToString(user_id) +
                         "' not exists") {}

UserStorage::UserStorage(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      cluster_(context.FindComponent<userver::components::Postgres>("database")
                   .GetCluster()) {}

boost::uuids::uuid UserStorage::CreateUser(const User& user) {
  try {
    auto transaction = cluster_->Begin(
        "create_user", userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Transaction::RW);

    const auto result = transaction.Execute(
        queries::sql::kCreateUser, user.login, user.first_name, user.last_name);

    transaction.Commit();

    return result.AsSingleRow<boost::uuids::uuid>();
  } catch (const userver::storages::postgres::UniqueViolation& e) {
    throw UserAlreadyExistsException(user.login);
  }
}

User UserStorage::GetUser(const boost::uuids::uuid& user_id) const {
  const auto result =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                        queries::sql::kGetUserById, user_id);

  if (result.IsEmpty()) {
    throw UserNotFound(user_id);
  }

  return result.AsSingleRow<User>();
}

std::vector<boost::uuids::uuid> UserStorage::FindUsers(
    const std::optional<std::string>& login_pattern,
    const std::optional<std::string>& first_name_pattern,
    const std::optional<std::string>& last_name_pattern) const {
  const auto result =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                        queries::sql::kFindUsers, login_pattern,
                        first_name_pattern, last_name_pattern);

  return result.AsContainer<std::vector<boost::uuids::uuid>>();
}

}  // namespace components::user_storage
