#pragma once

#include <stdexcept>
#include <string>
#include <unordered_set>
#include <userver/components/component_base.hpp>

namespace components::user_storage {

struct User {
  std::string id;  // TODO: Remove ID from document
  std::string login;
  std::string firstName;
  std::string lastName;
  std::unordered_set<std::string> propertyIds;
};

class UserAlreadyExistsException : public std::runtime_error {
 public:
  explicit UserAlreadyExistsException(const std::string& user_id);
};

class UserNotFound : public std::runtime_error {
 public:
  explicit UserNotFound(const std::string& user_id);
};

class UserStorage final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "user-storage";

  explicit UserStorage(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);

  void CreateUser(const User& user);

  void UpdateUser(const User& user);

  User GetUser(const std::string& user_id) const;

 private:
  std::unordered_map<std::string, User> users_;
};

}  // namespace components::user_storage
