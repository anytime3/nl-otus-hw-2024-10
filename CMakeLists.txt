cmake_minimum_required(VERSION 3.30)
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})
project(http_server VERSION ${PROJECT_VESRION})
set(CMAKE_CXX_STANDARD 17)


find_package(spdlog REQUIRED)
find_package(PostgreSQL REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

add_library(db_lib STATIC src/DbMng.cpp)
set_target_properties(db_lib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(db_lib PUBLIC PostgreSQL::PostgreSQL ${PQXX_LINK_LIBRARIES} spdlog::spdlog)

add_library(authMng_lib STATIC src/AuthMng.cpp)
set_target_properties(authMng_lib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(authMng_lib PUBLIC db_lib)

add_library(contentMng_lib STATIC src/ContentMng.cpp)
set_target_properties(contentMng_lib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(contentMng_lib PUBLIC db_lib)

find_package(Boost COMPONENTS system json REQUIRED)
set(Boost_USE_STATIC_LIBS ON)

find_package(OpenSSL REQUIRED)

find_package(GTest REQUIRED)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  message(STATUS "Boost found: ${Boost_VERSION_STRING}")
else()
    message(FATAL_ERROR "Boost not found")
endif()


add_library(requestMng_lib STATIC src/RequestMng.cpp)
set_target_properties(requestMng_lib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(requestMng_lib PUBLIC contentMng_lib authMng_lib Boost::boost Boost::system Boost::json)

add_library(beast_server_lib STATIC src/HttpServer.cpp)
set_target_properties(beast_server_lib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(beast_server_lib PUBLIC requestMng_lib OpenSSL::SSL OpenSSL::Crypto)

add_executable(http_server main.cpp)
target_link_libraries(http_server beast_server_lib)

add_executable(test_http_server tests/tests.cpp)
include_directories("src/")
target_link_libraries(test_http_server beast_server_lib GTest::GTest GTest::Main)
set(CMAKE_TESTING_ENABLED true)
enable_testing()
add_test(unit_tests test_http_server)

install(TARGETS http_server RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)
