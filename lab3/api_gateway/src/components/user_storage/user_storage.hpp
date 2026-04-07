#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <userver/components/component_base.hpp>
#include <userver/storages/postgres/cluster.hpp>

namespace components::user_storage {

struct User {
  std::string login;
  std::string first_name;
  std::string last_name;
};

class UserAlreadyExistsException : public std::runtime_error {
 public:
  explicit UserAlreadyExistsException(const std::string& login);
};

class UserNotFound : public std::runtime_error {
 public:
  explicit UserNotFound(const boost::uuids::uuid& user_id);
};

class UserStorage final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "user-storage";

  explicit UserStorage(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);

  boost::uuids::uuid CreateUser(const User& user);

  std::vector<boost::uuids::uuid> FindUsers(
      const std::optional<std::string>& login_pattern,
      const std::optional<std::string>& first_name_pattern,
      const std::optional<std::string>& last_name_pattern) const;

  User GetUser(const boost::uuids::uuid& user_id) const;

 private:
  userver::storages::postgres::ClusterPtr cluster_;
};

}  // namespace components::user_storage
