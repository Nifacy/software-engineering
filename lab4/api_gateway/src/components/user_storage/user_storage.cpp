#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <components/serializers.hpp>
#include <components/user_storage/user_storage.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/mongo/exception.hpp>
#include <userver/utils/boost_uuid4.hpp>

namespace userver::formats::parse {

using User = ::components::user_storage::User;

User Parse(const bson::Value& document, To<User>) {
  return User{
      .login = document["login"].As<std::string>(),
      .first_name = document["first_name"].As<std::string>(),
      .last_name = document["last_name"].As<std::string>(),
  };
}

}  // namespace userver::formats::parse

namespace components::user_storage {

namespace mongo = userver::storages::mongo;
namespace bson = userver::formats::bson;

UserAlreadyExistsException::UserAlreadyExistsException(const std::string& login)
    : std::runtime_error("User with login '" + login + "' already exists") {}

UserNotFound::UserNotFound(const boost::uuids::uuid& user_id)
    : std::runtime_error("User with ID '" + userver::utils::ToString(user_id) +
                         "' not exists") {}

UserStorage::UserStorage(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-database")
                .GetPool()) {}

boost::uuids::uuid UserStorage::CreateUser(const User& user) {
  const auto user_id = userver::utils::generators::GenerateBoostUuid();
  const auto document =
      bson::MakeDoc("_id", user_id, "login", user.login, "first_name",
                    user.first_name, "last_name", user.last_name);
  auto collection = pool_->GetCollection("users");

  try {
    collection.InsertOne(document);
    return user_id;
  } catch (const mongo::DuplicateKeyException& e) {
    throw UserAlreadyExistsException(user.login);
  }
}

User UserStorage::GetUser(const boost::uuids::uuid& user_id) const {
  const auto collection = pool_->GetCollection("users");
  const auto result = collection.FindOne(bson::MakeDoc("_id", user_id));

  if (!result.has_value()) {
    throw UserNotFound(user_id);
  }

  return (*result).As<User>();
}

std::vector<boost::uuids::uuid> UserStorage::FindUsers(
    const std::optional<std::string>& login_pattern,
    const std::optional<std::string>& first_name_pattern,
    const std::optional<std::string>& last_name_pattern) const {
  bson::ValueBuilder builder;

  if (login_pattern.has_value())
    builder["login"] = bson::MakeDoc("$regex", *login_pattern);

  if (first_name_pattern.has_value())
    builder["first_name"] = bson::MakeDoc("$regex", *first_name_pattern);

  if (last_name_pattern.has_value())
    builder["last_name"] = bson::MakeDoc("$regex", *last_name_pattern);

  const auto filter =
      builder.IsEmpty() ? bson::MakeDoc() : builder.ExtractValue();

  const auto collection = pool_->GetCollection("users");
  auto result = collection.Find(filter);
  std::vector<boost::uuids::uuid> user_ids;

  for (const auto& doc : result) {
    user_ids.push_back(doc["_id"].As<boost::uuids::uuid>());
  }

  return user_ids;
}

}  // namespace components::user_storage
