cmake_minimum_required(VERSION 3.30)
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})
project(bulk_server VERSION ${PROJECT_VESRION})
set(CMAKE_CXX_STANDARD 17)

add_library(async STATIC
    CommandProcessor.cpp
)

find_package(Boost REQUIRED COMPONENTS system)
set(Boost_USE_STATIC_LIBS ON)

set_target_properties(async PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME "async"
)

add_executable(bulk_server main.cpp)


target_link_libraries(bulk_server PRIVATE async Boost::system)

install(TARGETS bulk_server RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)
