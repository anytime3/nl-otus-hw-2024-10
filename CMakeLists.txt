cmake_minimum_required(VERSION 3.23...3.24)

list(INSERT CMAKE_MODULE_PATH 0 "${HW3_SCRIPTS_CMAKE_DIR}")

if(DEFINED ENV{GITHUB_RUN_NUMBER})
  set(PATCH_NUM $ENV{GITHUB_RUN_NUMBER})
else()
  set(PATCH_NUM 0)
endif()

project(fashio_mnist
    LANGUAGES CXX
    VERSION 0.0.${PATCH_NUM}
)

set(CMAKE_CXX_STANDARD 17)

if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
message(STATUS "CMake build type = ${CMAKE_BUILD_TYPE} platform-dir=${PLATFORM_BUILD_DIR}")

configure_file(version.h.in version.h)

add_subdirectory(fashio_mnist)

option(BUILD_TESTS "Build unittests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

option(GENERATE_DEB ON)
message(STATUS "Generate DEB file = ${GENERATE_DEB}")
if(GENERATE_DEB)
  set(CPACK_GENERATOR DEB)
  set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
  set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
  set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
  set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_SOURCE_DIR}/output/${PLATFORM_BUILD_DIR}/packages")
  set(CPACK_PACKAGE_CONTACT ardanew2@gmail.com)
  set(CPACK_DEB_COMPONENT_INSTALL ON)
  set(CPACK_COMPONENTS_ALL app) # to exclude tests
  set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
  include(CPack)
endif()

