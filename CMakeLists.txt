cmake_minimum_required(VERSION 3.10)
project(bayan)

set(CMAKE_CXX_STANDARD 17)


find_package(Boost 1.66 REQUIRED COMPONENTS filesystem program_options)
set(Boost_USE_STATIC_LIBS ON)

add_executable(bayan main.cpp)


target_link_libraries(bayan
    PRIVATE
        Boost::filesystem
        Boost::program_options
)

install(TARGETS bayan RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)
