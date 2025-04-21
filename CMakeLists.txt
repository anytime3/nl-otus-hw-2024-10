cmake_minimum_required(VERSION 3.30)
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})
project(http_server VERSION ${PROJECT_VESRION})
set(CMAKE_CXX_STANDARD 17)


find_package(Boost 1.85 REQUIRED COMPONENTS program_options system)
set(Boost_USE_STATIC_LIBS ON)

add_library(beast_server_lib STATIC src/HttpServer.cpp)
set_target_properties(beast_server_lib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(beast_server_lib PRIVATE Boost::system Boost::program_options)

add_executable(http_server main.cpp)


target_link_libraries(http_server beast_server_lib)

install(TARGETS http_server RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
include(CPack)
