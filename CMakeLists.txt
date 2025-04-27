cmake_minimum_required(VERSION 3.30)
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})
project(http_server VERSION ${PROJECT_VESRION})
set(CMAKE_CXX_STANDARD 17)


find_package(PostgreSQL REQUIRED)

add_library(db_lib STATIC src/DbMng.cpp)
set_target_properties(db_lib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(db_lib PUBLIC PostgreSQL::PostgreSQL)

add_library(authMng_lib STATIC src/AuthMng.cpp)
set_target_properties(authMng_lib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(authMng_lib PUBLIC db_lib)

add_library(contentMng_lib STATIC src/ContentMng.cpp)
set_target_properties(contentMng_lib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(contentMng_lib PUBLIC db_lib)

find_package(Boost 1.85 REQUIRED COMPONENTS system json)
set(Boost_USE_STATIC_LIBS ON)

find_package(OpenSSL REQUIRED)


if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  message(STATUS "Boost found: ${Boost_VERSION_STRING}")
else()
    message(FATAL_ERROR "Boost not found")
endif()

add_library(beast_server_lib STATIC src/HttpServer.cpp)
set_target_properties(beast_server_lib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(beast_server_lib PUBLIC authMng_lib contentMng_lib OpenSSL::SSL OpenSSL::Crypto Boost::boost Boost::system Boost::json)

add_executable(http_server main.cpp)
target_link_libraries(http_server beast_server_lib)

install(TARGETS http_server RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
include(CPack)
