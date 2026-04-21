cmake_minimum_required(VERSION 3.12...3.31)
project(api_gateway CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_PREFIX_PATH "/usr/lib/x86_64-linux-gnu/cmake")

include(DownloadUserver)

find_package(
    userver
    COMPONENTS core postgresql chaotic mongo#
    QUIET
)
if(NOT userver_FOUND)
    # Tries TRY_DIR first, falls back to downloading userver from GitHub using CPM.
    download_userver(TRY_DIR third_party/userver)
endif()

# Install jwt-cpp package
find_package(jwt-cpp QUIET)
if (NOT jwt-cpp_FOUND)
    message("Loading JWT-CPP")
    include(cmake/get_cpm.cmake)

    CPMAddPackage(
        NAME jwt-cpp
        GITHUB_REPOSITORY Thalhammer/jwt-cpp
        VERSION 0.7.2
        OPTIONS "JWT_BUILD_EXAMPLES OFF" "JWT_BUILD_TESTS OFF" "JWT_CMAKE_FILES_INSTALL ON"
    )
endif()

userver_setup_environment()

# Common sources

include_directories(src)

file(GLOB_RECURSE COMPONENT_SOURCE_FILES "src/components/*.cpp")
file(GLOB_RECURSE HANDLER_SOURCE_FILES "src/handlers/*.cpp")
file(GLOB_RECURSE AUTH_SOURCE_FILES "src/auth/*.cpp")

add_library(
    ${PROJECT_NAME}_objs OBJECT
    ${COMPONENT_SOURCE_FILES}
    ${HANDLER_SOURCE_FILES}
    ${AUTH_SOURCE_FILES}
)

target_link_libraries(
    ${PROJECT_NAME}_objs
    PUBLIC userver::core #
    PUBLIC jwt-cpp::jwt-cpp #
    PUBLIC userver::postgresql #
    PUBLIC userver::mongo #
)

# Generate schema C++ types

file(GLOB SCHEMA_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/api/definitions/*.yaml")
file(COPY ${SCHEMA_SOURCE_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/schemas)

file(GLOB_RECURSE SCHEMAS ${CMAKE_CURRENT_BINARY_DIR}/schemas/*.yaml)
userver_target_generate_chaotic(
    ${PROJECT_NAME}-chgen
    # Generate serializers for responses
    GENERATE_SERIALIZERS
    LAYOUT "/components/schemas/([^/]*)/=api_gateway::schemas::{stem}::{0}"
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src/schemas
    SCHEMAS ${SCHEMAS}
    RELATIVE_TO ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(${PROJECT_NAME}_objs PRIVATE ${PROJECT_NAME}-chgen)

# Generate static files

file(GLOB OPEN_API_FILES "${CMAKE_CURRENT_SOURCE_DIR}/api/*")
file(COPY ${OPEN_API_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/static/docs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/static/swagger.html DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/static/docs/)
file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/static/docs/swagger.html ${CMAKE_CURRENT_BINARY_DIR}/static/docs/index.html)

# Build SQL queries
userver_add_sql_library(
    ${PROJECT_NAME}-sql
    NAMESPACE
    queries
    SOURCE_DIR
    postgresql/queries
    OUTPUT_DIR
    ${CMAKE_CURRENT_BINARY_DIR}
    SQL_FILES
    *.sql
)
target_link_libraries(${PROJECT_NAME}_objs PRIVATE ${PROJECT_NAME}-sql)

# The Service

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

# Functional testing

userver_testsuite_add_simple()
