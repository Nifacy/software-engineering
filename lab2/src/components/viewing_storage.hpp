#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <userver/components/component_base.hpp>

namespace components::viewing_storage {

struct Date {
  int year;
  int month;
  int day;

  bool operator==(const Date& other) const;
};

struct Viewing {
  std::string user_id;
  std::string property_id;
  Date date;
};

class ViewingAlreadyExists : public std::runtime_error {
 public:
  explicit ViewingAlreadyExists(const std::string& viewing_id);
};

class ViewingNotFound : public std::runtime_error {
 public:
  explicit ViewingNotFound(const std::string& viewing_id);
};

class ViewingStorage final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "viewing-storage";

  explicit ViewingStorage(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context);

  void CreateViewing(const std::string& viewing_id, const Viewing& viewing);
  void DeleteViewing(const std::string& viewing_id);

  Viewing GetViewing(const std::string& viewing_id) const;

  std::vector<std::string> FindViewings(
      const std::optional<std::string>& user_id,
      const std::string& property_id) const;

  std::vector<std::string> FindViewings(
      const std::string& user_id,
      const std::optional<std::string>& property_id) const;

 private:
  std::vector<std::string> FindViewingsInternal(
      const std::optional<std::string>& user_id,
      const std::optional<std::string>& property_id) const;

  std::unordered_map<std::string, Viewing> storage_;
};

}  // namespace components::viewing_storage
