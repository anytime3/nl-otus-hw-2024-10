set(TARGET_NAME fashio_mnist)

set(SERVER_SOURCES 
    CACHE FILEPATH ""
)

add_executable(${TARGET_NAME} main.cpp)

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
if(MSVC)
    target_compile_definitions(${TARGET_NAME} PRIVATE _WIN32_WINNT=0x0601)
endif()

if(WIN32)
    add_library(catboostmodel STATIC IMPORTED)
    set_property(TARGET catboostmodel PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/contrib/catboostmodel.lib")
else()
    add_library(catboostmodel SHARED IMPORTED)
    set_property(TARGET catboostmodel PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/contrib/libcatboostmodel.so")
ENDIF()

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/contrib)
target_link_libraries(${TARGET_NAME} PRIVATE catboostmodel)

include(GNUInstallDirs)
install(
    TARGETS ${TARGET_NAME} COMPONENT app
    PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(WIN32)
    install(FILES ${CMAKE_SOURCE_DIR}/contrib/catboostmodel.dll DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(FILES ${CMAKE_SOURCE_DIR}/contrib/libcatboostmodel.so DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()



install(FILES ${CMAKE_SOURCE_DIR}/contrib/model.cbm DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_SOURCE_DIR}/contrib/test_data_catboost.txt DESTINATION ${CMAKE_INSTALL_BINDIR})
