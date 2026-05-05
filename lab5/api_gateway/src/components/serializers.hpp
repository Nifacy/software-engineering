#pragma once

#include <boost/uuid/uuid.hpp>
#include <userver/formats/bson.hpp>
#include <userver/formats/json.hpp>
#include <userver/utils/boost_uuid4.hpp>

namespace userver::formats::parse {

inline boost::uuids::uuid Parse(const bson::Value& value,
                                To<boost::uuids::uuid>) {
  return utils::BoostUuidFromString(value.As<std::string>());
}

}  // namespace userver::formats::parse

namespace userver::formats::serialize {

inline bson::Value Serialize(const boost::uuids::uuid& value, To<bson::Value>) {
  return bson::MakeDoc("value", utils::ToString(value))["value"];
}

}  // namespace userver::formats::serialize

namespace boost::uuids {

// TODO: migrate on generic implementation
inline uuid Parse(const userver::formats::json::Value& value,
                  userver::formats::parse::To<boost::uuids::uuid>) {
  return userver::utils::BoostUuidFromString(value.As<std::string>());
}

inline userver::formats::json::Value Serialize(
    const uuid& value,
    userver::formats::serialize::To<userver::formats::json::Value>) {
  return userver::formats::json::MakeObject(
      "value", userver::utils::ToString(value))["value"];
}

}  // namespace boost::uuids

namespace std {

template <typename A, typename B>
pair<A, B> Parse(const userver::formats::json::Value& value,
                 userver::formats::parse::To<std::pair<A, B>>) {
  value.CheckArray();
  return {value[0].As<A>(), value[1].As<B>()};
}

template <typename A, typename B>
userver::formats::json::Value Serialize(
    const pair<A, B>& value,
    userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder.PushBack(value.first);
  builder.PushBack(value.second);

  return builder.ExtractValue();
}

}  // namespace std