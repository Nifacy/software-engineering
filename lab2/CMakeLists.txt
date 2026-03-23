cmake_minimum_required(VERSION 3.12...3.31)
project(api_gateway CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)

find_package(
    userver
    COMPONENTS core chaotic#
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

add_library(
    ${PROJECT_NAME}_objs OBJECT
    src/components/credentials_storage.cpp
    src/components/user_storage.cpp
    src/components/property_storage.cpp
    src/components/viewing_storage.cpp
    src/handlers/register_handler.cpp
    src/handlers/login_handler.cpp
    src/auth/auth_checker.cpp
    src/auth/auth_checker_factory.cpp
    src/handlers/get_current_user_handler.cpp
    src/handlers/create_property_handler.cpp
    src/handlers/find_users_handler.cpp
    src/handlers/get_user_handler.cpp
    src/handlers/find_properties_handler.cpp
    src/handlers/get_property_handler.cpp
    src/handlers/schedule_viewing_handler.cpp
    src/handlers/delete_viewing_handler.cpp
)

target_link_libraries(
    ${PROJECT_NAME}_objs
    PUBLIC userver::core
)


# Generate schema C++ types

file(GLOB_RECURSE SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.yaml)
userver_target_generate_chaotic(
    ${PROJECT_NAME}-chgen
    # Generate serializers for responses
    GENERATE_SERIALIZERS
    # Map '/components/schemas/*' JSONSchema types to C++ types in 'api_gateway::schemas::STEM' namespace
    LAYOUT "/components/schemas/([^/]*)/=api_gateway::schemas::{stem}::{0}"
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
    SCHEMAS ${SCHEMAS}
    RELATIVE_TO ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${PROJECT_NAME}_objs PRIVATE ${PROJECT_NAME}-chgen)


# The Service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

# Functional testing
userver_testsuite_add_simple()

# Install
include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
    message(STATUS "Set install prefix: $ENV{PREFIX}")
    file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
    set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT ${PROJECT_NAME}
)
install(
    FILES ${CONFIGS_FILES}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
)