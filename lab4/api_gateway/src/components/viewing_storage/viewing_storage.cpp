#include <components/viewing_storage/viewing_storage.hpp>
#include <queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/date.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime.hpp>

namespace components::viewing_storage {

ViewingNotFound::ViewingNotFound(const boost::uuids::uuid& viewing_id)
    : std::runtime_error("Viewing with ID '" +
                         userver::utils::ToString(viewing_id) + "' not found") {
}

ViewingDateTaken::ViewingDateTaken()
    : std::runtime_error("Viewing date already taken") {}

ViewingStorage::ViewingStorage(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      cluster_(context.FindComponent<userver::components::Postgres>("database")
                   .GetCluster()) {}

boost::uuids::uuid ViewingStorage::CreateViewing(const Viewing& viewing) {
  auto transaction = cluster_->Begin(
      "create_viewing", userver::storages::postgres::ClusterHostType::kMaster,
      userver::storages::postgres::Transaction::RW);

  try {
    const auto result =
        transaction.Execute(queries::sql::kCreateViewing, viewing.user_id,
                            viewing.property_id, viewing.viewing_date);
    transaction.Commit();
    return result.AsSingleRow<boost::uuids::uuid>();
  } catch (const userver::storages::postgres::UniqueViolation&) {
    throw ViewingDateTaken();
  }
}

void ViewingStorage::DeleteViewing(const boost::uuids::uuid& viewing_id) {
  auto transaction = cluster_->Begin(
      "delete_viewing", userver::storages::postgres::ClusterHostType::kMaster,
      userver::storages::postgres::Transaction::RW);

  const auto result =
      transaction.Execute(queries::sql::kDeleteViewing, viewing_id);

  if (result.IsEmpty()) {
    throw ViewingNotFound(viewing_id);
  }

  transaction.Commit();
}

Viewing ViewingStorage::GetViewing(const boost::uuids::uuid& viewing_id) const {
  const auto result =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                        queries::sql::kGetViewingById, viewing_id);

  if (result.IsEmpty()) {
    throw ViewingNotFound(viewing_id);
  }

  return result.AsSingleRow<Viewing>();
}

std::vector<boost::uuids::uuid> ViewingStorage::FindViewingsInternal(
    const std::optional<boost::uuids::uuid>& user_id,
    const std::optional<boost::uuids::uuid>& property_id) const {
  const auto result =
      cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                        queries::sql::kFindViewings, user_id, property_id);

  return result.AsContainer<std::vector<boost::uuids::uuid>>();
}

std::vector<boost::uuids::uuid> ViewingStorage::FindViewings(
    const std::optional<boost::uuids::uuid>& user_id,
    const boost::uuids::uuid& property_id) const {
  return FindViewingsInternal(user_id, property_id);
}

std::vector<boost::uuids::uuid> ViewingStorage::FindViewings(
    const boost::uuids::uuid& user_id,
    const std::optional<boost::uuids::uuid>& property_id) const {
  return FindViewingsInternal(user_id, property_id);
}

}  // namespace components::viewing_storage
