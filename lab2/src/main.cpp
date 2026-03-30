#include <auth/auth_checker_factory.hpp>
#include <components/credentials_storage.hpp>
#include <components/jwt_auth.hpp>
#include <components/viewing_storage.hpp>
#include <handlers/delete_viewing_handler.hpp>
#include <handlers/find_properties_handler.hpp>
#include <handlers/find_users_handler.hpp>
#include <handlers/get_current_user_handler.hpp>
#include <handlers/get_property_handler.hpp>
#include <handlers/get_property_viewings_handler.hpp>
#include <handlers/get_user_handler.hpp>
#include <handlers/get_user_properties_handler.hpp>
#include <handlers/get_user_viewings_handler.hpp>
#include <handlers/login_handler.hpp>
#include <handlers/refresh_token_handler.hpp>
#include <handlers/register_handler.hpp>
#include <handlers/schedule_viewing_handler.hpp>
#include <handlers/update_property_handler.hpp>
#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/fs_cache.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/auth/auth_checker_factory.hpp>
#include <userver/server/handlers/http_handler_static.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>
#include "components/property_storage.hpp"
#include "components/user_storage.hpp"
#include "handlers/create_property_handler.hpp"

int main(int argc, char* argv[]) {
  userver::server::handlers::auth::RegisterAuthCheckerFactory<
      auth::AuthCheckerFactory>();

  auto component_list =
      userver::components::MinimalServerComponentList()
          .Append<userver::components::TestsuiteSupport>()
          .AppendComponentList(userver::clients::http::ComponentList())
          .Append<userver::components::FsCache>("fs-cache-main")
          .Append<userver::clients::dns::Component>()
          .Append<userver::server::handlers::TestsControl>()
          .Append<userver::congestion_control::Component>()
          .Append<components::user_storage::UserStorage>()
          .Append<components::property_storage::PropertyStorage>()
          .Append<components::jwt_auth::JwtAuthComponent>()
          .Append<components::credentials_storage::CredentialsStorage>()
          .Append<components::viewing_storage::ViewingStorage>()
          .Append<handlers::register_handler::RegisterHandler>()
          .Append<handlers::login_handler::LoginHandler>()
          .Append<auth::AuthCheckerComponent>()
          .Append<handlers::get_current_user_handler::GetCurrentUserHandler>()
          .Append<handlers::create_property_handler::CreatePropertyHandler>()
          .Append<handlers::find_users_handler::FindUsersHandler>()
          .Append<handlers::get_user_handler::GetUserHandler>()
          .Append<handlers::find_properties_handler::FindPropertiesHandler>()
          .Append<handlers::get_property_handler::GetPropertyHandler>()
          .Append<handlers::schedule_viewing_handler::ScheduleViewingHandler>()
          .Append<handlers::delete_viewing_handler::DeleteViewingHandler>()
          .Append<handlers::get_property_viewings_handler::
                      GetPropertyViewingsHandler>()
          .Append<handlers::get_user_viewings_handler::GetUserViewingsHandler>()
          .Append<handlers::update_property_handler::UpdatePropertyHandler>()
          .Append<
              handlers::get_user_properties_handler::GetUserPropertiesHandler>()
          .Append<handlers::refresh_token_handler::RefreshTokenHandler>()
          .Append<userver::server::handlers::HttpHandlerStatic>("docs-handler");

  return userver::utils::DaemonMain(argc, argv, component_list);
}
