#pragma once

#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers::static_file_handler {

class StaticFileHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "static-file-handler";

  StaticFileHandler(const userver::components::ComponentConfig& config,
                    const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

  static userver::yaml_config::Schema GetStaticConfigSchema();

 private:
  std::string file_content_;
  std::string content_type_;
};

}  // namespace handlers::static_file_handler