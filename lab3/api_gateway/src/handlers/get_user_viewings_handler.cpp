#include <handlers/get_user_viewings_handler.hpp>
#include <schemas/user.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime/date.hpp>

namespace handlers::get_user_viewings_handler {

api_gateway::schemas::user::UserViewing SerializeViewing(
    const std::string& viewing_id,
    const components::viewing_storage::Viewing& viewing) {
  return {
      .propertyId = viewing.property_id,
      .id = viewing_id,
      .date = userver::utils::datetime::DateFromRFC3339String(viewing.date),
  };
}

GetUserViewingsHandler::GetUserViewingsHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : SchemaHttpHandler(config, context),
      viewing_storage_(
          context
              .FindComponent<components::viewing_storage::ViewingStorage>()) {}

common::Response GetUserViewingsHandler::HandleRequestImpl(
    const userver::server::http::HttpRequest&,
    userver::server::request::RequestContext& request_context) const {
  const auto user_id = request_context.GetData<boost::uuids::uuid>("user_id");
  api_gateway::schemas::user::UserViewingList response_dom;

  for (const auto& viewing_id : viewing_storage_.FindViewings(
           userver::utils::ToString(user_id), std::nullopt)) {
    const auto viewing = viewing_storage_.GetViewing(viewing_id);
    response_dom.viewings.push_back(SerializeViewing(viewing_id, viewing));
  }

  return common::Response(userver::http::StatusCode::OK, response_dom);
}

}  // namespace handlers::get_user_viewings_handler