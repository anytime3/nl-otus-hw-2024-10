cmake_minimum_required(VERSION 3.30)
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})
project(editor VERSION ${PROJECT_VESRION})



set(CMAKE_CXX_STANDARD 17)

add_library(view STATIC libs/View.cpp)
add_library(controller STATIC libs/Controller.cpp)
add_library(document STATIC libs/Document.cpp)
add_library(primitive STATIC libs/Primitive.cpp)

set_target_properties(view PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(controller PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(document PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(primitive PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(document primitive)
target_link_libraries(controller document view)
target_link_libraries(view document)

add_executable(editor
              main.cpp
)
target_link_libraries(editor view controller document primitive)


install(TARGETS editor RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)
