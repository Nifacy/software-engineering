#pragma once

#include <boost/uuid/uuid.hpp>
#include <userver/formats/bson.hpp>
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
