#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "components/credentials_storage.hpp"
#include "components/user_storage.hpp"

namespace handlers::register_handler {

class RegisterHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "register-handler";

  RegisterHandler(const userver::components::ComponentConfig& config,
                  const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  components::credentials_storage::CredentialsStorage& credentials_storage_;
  components::user_storage::UserStorage& user_storage_;
};

}  // namespace handlers::register_handler