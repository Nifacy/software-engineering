#pragma once

#include <boost/uuid/uuid.hpp>
#include <userver/formats/bson.hpp>
#include <userver/formats/json.hpp>
#include <userver/utils/boost_uuid4.hpp>

namespace boost::uuids {

template <class Value>
uuid Parse(const Value& data, userver::formats::parse::To<uuid>) {
  return userver::utils::BoostUuidFromString(data.template As<std::string>());
}

template <class Value>
Value Serialize(const uuid& data, userver::formats::serialize::To<Value>) {
  typename Value::Builder builder;
  builder["value"] = userver::utils::ToString(data);
  return builder.ExtractValue()["value"];
}

}  // namespace boost::uuids

namespace std {

template <class Value, typename A, typename B>
pair<A, B> Parse(const Value& value,
                 userver::formats::parse::To<std::pair<A, B>>) {
  value.CheckArray();
  return {value[0].template As<A>(), value[1].template As<B>()};
}

template <class Value, typename A, typename B>
Value Serialize(const pair<A, B>& value,
                userver::formats::serialize::To<Value>) {
  typename Value::Builder builder;
  builder.PushBack(value.first);
  builder.PushBack(value.second);
  return builder.ExtractValue();
}

}  // namespace std
