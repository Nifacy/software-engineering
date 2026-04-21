#include <boost/uuid/uuid.hpp>
#include <components/credentials_storage/credentials_storage.hpp>
#include <components/serializers.hpp>
#include <queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/bson.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/mongo/mongo_error.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/boost_uuid4.hpp>

namespace userver::formats::parse {

using Credentials = ::components::credentials_storage::Credentials;

Credentials Parse(const bson::Value& document, To<Credentials>) {
  return Credentials{
      .verify_secret = document["verify_secret"].As<std::string>(),
      .user_id = document["user_id"].As<boost::uuids::uuid>(),
  };
}
}  // namespace userver::formats::parse

namespace components::credentials_storage {

namespace mongo = userver::storages::mongo;
namespace bson = userver::formats::bson;

CredentialsAlreadyExists::CredentialsAlreadyExists(const std::string& key)
    : std::runtime_error("Credentials by key '" + key + "' already exist") {}

CredentialsNotFound::CredentialsNotFound(const std::string& key)
    : std::runtime_error("Credentials by key '" + key + "' not found") {}

InvalidVerifySecret::InvalidVerifySecret(const std::string& key)
    : std::runtime_error(
          "Passed invalid verify secret for credentials by key '" + key + "'") {
}

CredentialsStorage::CredentialsStorage(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-database")
                .GetPool()) {}

void CredentialsStorage::AddCredentials(const std::string& key,
                                        const Credentials& credentials) {
  try {
    auto collection = pool_->GetCollection("credentials");
    const auto document =
        bson::MakeDoc("_id", key, "verify_secret", credentials.verify_secret,
                      "user_id", credentials.user_id);

    collection.InsertOne(document);
  } catch (const mongo::MongoError& e) {
    if (e.GetKind() == mongo::MongoError::Kind::kDuplicateKey) {
      throw CredentialsAlreadyExists(key);
    } else {
      throw e;
    }
  }
}

boost::uuids::uuid CredentialsStorage::VerifyCredentials(
    const std::string& key, const std::string& verify_secret) const {
  auto collection = pool_->GetCollection("credentials");
  const auto result = collection.FindOne(bson::MakeDoc("_id", key));

  if (!result.has_value()) {
    throw CredentialsNotFound(key);
  }

  auto credentials = (*result).As<Credentials>();
  if (credentials.verify_secret != verify_secret) {
    throw InvalidVerifySecret(key);
  }

  return credentials.user_id;
}

}  // namespace components::credentials_storage