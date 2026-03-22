#include <auth/auth_checker_factory.hpp>
#include <components/credentials_storage.hpp>
#include <handlers/info_handler.hpp>
#include <handlers/login_handler.hpp>
#include <handlers/register_handler.hpp>
#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/auth/auth_checker_factory.hpp>
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
          .Append<userver::clients::dns::Component>()
          .Append<userver::server::handlers::TestsControl>()
          .Append<userver::congestion_control::Component>()
          .Append<components::user_storage::UserStorage>()
          .Append<components::property_storage::PropertyStorage>()
          .Append<components::credentials_storage::CredentialsStorage>()
          .Append<handlers::register_handler::RegisterHandler>()
          .Append<handlers::login_handler::LoginHandler>()
          .Append<auth::AuthCheckerComponent>()
          .Append<handlers::info_handler::CredentialsInfoHandler>()
          .Append<handlers::create_property_handler::CreatePropertyHandler>();

  return userver::utils::DaemonMain(argc, argv, component_list);
}
