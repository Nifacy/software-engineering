#include <components/serializers.hpp>
#include <components/viewing_storage/viewing_storage.hpp>
#include <queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/bson.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/mongo/exception.hpp>
#include <userver/storages/mongo/mongo_error.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime.hpp>

namespace userver::formats::parse {

using Viewing = ::components::viewing_storage::Viewing;

Viewing Parse(const bson::Value& document, To<Viewing>) {
  return Viewing{
      .user_id = document["user_id"].As<boost::uuids::uuid>(),
      .property_id = document["property_id"].As<boost::uuids::uuid>(),
      .viewing_date =
          document["viewing_date"].As<userver::utils::datetime::Date>(),
  };
}
}  // namespace userver::formats::parse

namespace components::viewing_storage {

namespace mongo = userver::storages::mongo;
namespace bson = userver::formats::bson;

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
      pool_(context.FindComponent<userver::components::Mongo>("mongo-database")
                .GetPool()) {}

boost::uuids::uuid ViewingStorage::CreateViewing(const Viewing& viewing) {
  const auto viewing_id = userver::utils::generators::GenerateBoostUuid();

  const auto document = bson::MakeDoc(
      "_id", viewing_id, "user_id", viewing.user_id, "property_id",
      viewing.property_id, "viewing_date", viewing.viewing_date);

  try {
    auto collection = pool_->GetCollection("viewings");
    collection.InsertOne(document);
    return viewing_id;
  } catch (const mongo::DuplicateKeyException& e) {
    throw ViewingDateTaken();
  }
}

void ViewingStorage::DeleteViewing(const boost::uuids::uuid& viewing_id) {
  const auto document = bson::MakeDoc("_id", viewing_id);
  auto collection = pool_->GetCollection("viewings");
  const auto result = collection.DeleteOne(document);

  if (result.DeletedCount() == 0) {
    throw ViewingNotFound(viewing_id);
  }
}

Viewing ViewingStorage::GetViewing(const boost::uuids::uuid& viewing_id) const {
  const auto document = bson::MakeDoc("_id", viewing_id);
  auto collection = pool_->GetCollection("viewings");
  const auto result = collection.FindOne(document);

  if (!result.has_value()) {
    throw ViewingNotFound(viewing_id);
  }

  return (*result).As<Viewing>();
}

std::vector<boost::uuids::uuid> ViewingStorage::FindViewingsInternal(
    const std::optional<boost::uuids::uuid>& user_id,
    const std::optional<boost::uuids::uuid>& property_id) const {
  bson::ValueBuilder builder;

  if (user_id.has_value()) {
    builder["user_id"] = *user_id;
  }

  if (property_id.has_value()) {
    builder["property_id"] = *property_id;
  }

  auto collection = pool_->GetCollection("viewings");
  auto result = collection.Find(builder.ExtractValue());
  std::vector<boost::uuids::uuid> viewing_ids;

  for (const auto& document : result) {
    viewing_ids.push_back(document["_id"].As<boost::uuids::uuid>());
  }

  return viewing_ids;
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
