#pragma once

#include <boost/uuid/uuid.hpp>
#include <optional>
#include <stdexcept>
#include <userver/components/component_base.hpp>
#include <userver/storages/mongo/pool.hpp>
#include <userver/utils/datetime/date.hpp>

namespace components::viewing_storage {

struct Viewing {
  boost::uuids::uuid user_id;
  boost::uuids::uuid property_id;
  userver::utils::datetime::Date viewing_date;
};

class ViewingNotFound : public std::runtime_error {
 public:
  explicit ViewingNotFound(const boost::uuids::uuid& viewing_id);
};

class ViewingDateTaken : public std::runtime_error {
 public:
  explicit ViewingDateTaken();
};

class ViewingStorage final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "viewing-storage";

  explicit ViewingStorage(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context);

  boost::uuids::uuid CreateViewing(const Viewing& viewing);
  void DeleteViewing(const boost::uuids::uuid& viewing_id);

  Viewing GetViewing(const boost::uuids::uuid& viewing_id) const;

  std::vector<boost::uuids::uuid> FindViewings(
      const std::optional<boost::uuids::uuid>& user_id,
      const boost::uuids::uuid& property_id) const;

  std::vector<boost::uuids::uuid> FindViewings(
      const boost::uuids::uuid& user_id,
      const std::optional<boost::uuids::uuid>& property_id) const;

 private:
  std::vector<boost::uuids::uuid> FindViewingsInternal(
      const std::optional<boost::uuids::uuid>& user_id,
      const std::optional<boost::uuids::uuid>& property_id) const;

  userver::storages::mongo::PoolPtr pool_;
};

}  // namespace components::viewing_storage
