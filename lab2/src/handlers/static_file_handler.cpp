#include <filesystem>
#include <fstream>
#include <handlers/static_file_handler.hpp>
#include <sstream>
#include <userver/components/component_config.hpp>
#include <userver/engine/task/task_processor_fwd.hpp>
#include <userver/fs/read.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace handlers::static_file_handler {

std::string readFile(const std::filesystem::path& path) {
  std::ifstream file(path.string());
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file '" + path.string() + "'");
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

std::filesystem::path validatePath(const std::string& value) {
  auto absolute_path = std::filesystem::absolute(value);
  auto normalized_path = absolute_path.lexically_normal();

  if (!std::filesystem::exists(normalized_path)) {
    throw std::runtime_error("Path '" + normalized_path.string() +
                             "' not exists");
  }

  return normalized_path;
}

StaticFileHandler::StaticFileHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      file_content_(
          readFile(validatePath(config["file_path"].As<std::string>()))),
      content_type_(config["content_type"].As<std::string>()) {}

std::string StaticFileHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  request.SetResponseStatus(userver::server::http::HttpStatus::OK);
  request.GetHttpResponse().SetContentType(content_type_);
  return file_content_;
}

userver::yaml_config::Schema StaticFileHandler::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<HttpHandlerBase>(
      R"(
type: object
additionalProperties: false
description: Static file HTTP handler
properties:
  file_path:
    type: string
    description: Path to static file
  content_type:
    type: string
    description: Content type of response
)");
}

}  // namespace handlers::static_file_handler