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

User UserStorage::GetUser(const std::string& user_id) const {
  auto it = users_.find(user_id);
  if (it == users_.end()) {
    throw UserNotFound(user_id);
  }

  return it->second;
}

}  // namespace components::user_storage
