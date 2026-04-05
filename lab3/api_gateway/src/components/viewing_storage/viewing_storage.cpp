#include <components/viewing_storage/viewing_storage.hpp>

namespace components::viewing_storage {

ViewingAlreadyExists::ViewingAlreadyExists(const std::string& viewing_id)
    : std::runtime_error("Viewing with ID '" + viewing_id +
                         "' already exists") {}

ViewingNotFound::ViewingNotFound(const std::string& viewing_id)
    : std::runtime_error("Viewing with ID '" + viewing_id + "' not found") {}

ViewingStorage::ViewingStorage(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context), storage_() {}

void ViewingStorage::CreateViewing(const std::string& viewing_id,
                                   const Viewing& viewing) {
  if (storage_.find(viewing_id) != storage_.end()) {
    throw ViewingAlreadyExists(viewing_id);
  }

  storage_.emplace(viewing_id, std::move(viewing));
}

void ViewingStorage::DeleteViewing(const std::string& viewing_id) {
  auto it = storage_.find(viewing_id);
  if (it == storage_.end()) {
    throw ViewingNotFound(viewing_id);
  }
  storage_.erase(it);
}

Viewing ViewingStorage::GetViewing(const std::string& viewing_id) const {
  auto it = storage_.find(viewing_id);
  if (it == storage_.end()) {
    throw ViewingNotFound(viewing_id);
  }
  return it->second;
}

std::vector<std::string> ViewingStorage::FindViewingsInternal(
    const std::optional<std::string>& user_id,
    const std::optional<std::string>& property_id) const {
  std::vector<std::string> result;

  for (const auto& [id, viewing] : storage_) {
    bool match_user = !user_id.has_value() || (viewing.user_id == *user_id);
    bool match_property =
        !property_id.has_value() || (viewing.property_id == *property_id);

    if (match_user && match_property) {
      result.push_back(id);
    }
  }

  return result;
}

std::vector<std::string> ViewingStorage::FindViewings(
    const std::optional<std::string>& user_id,
    const std::string& property_id) const {
  return FindViewingsInternal(user_id, property_id);
}

std::vector<std::string> ViewingStorage::FindViewings(
    const std::string& user_id,
    const std::optional<std::string>& property_id) const {
  return FindViewingsInternal(user_id, property_id);
}

}  // namespace components::viewing_storage
