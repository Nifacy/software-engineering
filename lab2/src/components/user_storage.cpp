#include <components/user_storage.hpp>

namespace components::user_storage {

UserAlreadyExistsException::UserAlreadyExistsException(
    const std::string& user_id)
    : std::runtime_error("User with ID '" + user_id + "' already exists") {}

UserNotFound::UserNotFound(const std::string& user_id)
    : std::runtime_error("User with ID '" + user_id + "' not exists") {}

UserStorage::UserStorage(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : ComponentBase(config, context), users_() {}

void UserStorage::CreateUser(const User& user) {
  if (users_.find(user.id) != users_.end()) {
    throw UserAlreadyExistsException(user.id);
  }

  users_.emplace(user.id, std::move(user));
}

void UserStorage::UpdateUser(const User& user) {
  auto it = users_.find(user.id);
  if (it == users_.end()) {
    throw UserNotFound(user.id);
  }
  it->second = user;
}

User UserStorage::GetUser(const std::string& user_id) const {
  auto it = users_.find(user_id);
  if (it == users_.end()) {
    throw UserNotFound(user_id);
  }

  return it->second;
}

std::vector<std::string> UserStorage::FindUsers(
    const std::optional<std::string>& login_pattern,
    const std::optional<std::string>& first_name_pattern,
    const std::optional<std::string>& last_name_pattern) const {
  std::vector<std::string> result;

  for (const auto& [id, user] : users_) {
    bool match_login = !login_pattern.has_value() ||
                       (user.login.find(*login_pattern) != std::string::npos);

    bool match_first_name =
        !first_name_pattern.has_value() ||
        (user.firstName.find(*first_name_pattern) != std::string::npos);

    bool match_last_name =
        !last_name_pattern.has_value() ||
        (user.lastName.find(*last_name_pattern) != std::string::npos);

    if (match_login && match_first_name && match_last_name) {
      result.push_back(id);
    }
  }

  return result;
}

}  // namespace components::user_storage
