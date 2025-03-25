cmake_minimum_required(VERSION 3.30)
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})
project(bulk VERSION ${PROJECT_VESRION})


set(CMAKE_CXX_STANDARD 17)

add_library(cmd_processor STATIC CommandProcessor.cpp)


set_target_properties(cmd_processor PROPERTIES LINKER_LANGUAGE CXX)

add_executable(bulk
              main.cpp )

target_link_libraries(bulk cmd_processor)


install(TARGETS bulk RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)
