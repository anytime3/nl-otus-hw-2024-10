cmake_minimum_required(VERSION 3.30)
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})
project(matrix VERSION ${PROJECT_VESRION})


set(CMAKE_CXX_STANDARD 17)

# add_library(matrix_lib STATIC Matrix.h)


# set_target_properties(matrix_lib PROPERTIES LINKER_LANGUAGE CXX)



add_executable(matrix
              main.cpp
              Matrix.h

)
# target_link_libraries(matrix matrix_lib)


install(TARGETS matrix RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)
