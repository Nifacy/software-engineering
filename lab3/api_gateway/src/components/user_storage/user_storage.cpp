#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <components/user_storage/user_storage.hpp>
#include <queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/boost_uuid4.hpp>

namespace components::user_storage {

std::vector<std::string> parseResult(
    const userver::storages::postgres::ResultSet& result) {
  const auto uuids = result.AsContainer<std::vector<boost::uuids::uuid>>();
  std::vector<std::string> parsed;

  for (const auto& el : uuids) {
    parsed.push_back(userver::utils::ToString(el));
  }

  return parsed;
}

std::optional<boost::uuids::uuid> parseID(const std::string& data) {
  try {
    return userver::utils::BoostUuidFromString(data);
  } catch (const std::exception&) {
    return std::nullopt;
  }
}

UserAlreadyExistsException::UserAlreadyExistsException(const std::string& login)
    : std::runtime_error("User with login '" + login + "' already exists") {}

UserNotFound::UserNotFound(const std::string& user_id)
    : std::runtime_error("User with ID '" + user_id + "' not exists") {}

UserStorage::UserStorage(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      cluster_(context.FindComponent<userver::components::Postgres>("database")
                   .GetCluster()) {}

std::string UserStorage::CreateUser(const User& user) {
  try {
    auto transaction = cluster_->Begin(
        "create_user", userver::storages::postgres::ClusterHostType::kMaster,
        userver::storages::postgres::Transaction::RW);

    const auto result = transaction.Execute(
        queries::sql::kCreateUser, user.login, user.first_name, user.last_name);

    transaction.Commit();

    auto user_id = result.AsSingleRow<boost::uuids::uuid>();
    return boost::uuids::to_string(user_id);
  } catch (const userver::storages::postgres::UniqueViolation& e) {
    throw UserAlreadyExistsException(user.login);
  }
}

User UserStorage::GetUser(const std::string& user_id) const {
  try {
    const auto maybe_parsed_user_id = parseID(user_id);
    if (!maybe_parsed_user_id.has_value()) {
      throw UserNotFound(user_id);
    }

    const auto result =
        cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                          queries::sql::kGetUserById, *maybe_parsed_user_id);

    if (result.IsEmpty()) {
      throw UserNotFound(user_id);
    }

    return result.AsSingleRow<User>();
  } catch (const std::invalid_argument&) {
    throw UserNotFound(user_id);
  }
}

std::vector<std::string> UserStorage::FindUsers(
    const std::optional<std::string>& login_pattern,
    const std::optional<std::string>& first_name_pattern,
    const std::optional<std::string>& last_name_pattern) const {
  const auto result =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                        queries::sql::kFindUsers, login_pattern,
                        first_name_pattern, last_name_pattern);

  return parseResult(result);
}

}  // namespace components::user_storage
