set(TARGET_NAME join_server)

set(SERVER_SOURCES 
    builder.h builder.cpp
    isender.h server.h server.cpp
    command.h validator.h validator.cpp
    clientdata.h clientdatafactory.h clientdatafactory.cpp
    itables.h tables.h tables.cpp
    CACHE FILEPATH ""
)
find_package(Boost REQUIRED COMPONENTS system locale)

add_executable(${TARGET_NAME} main.cpp ${SERVER_SOURCES})

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
if(MSVC)
    target_compile_definitions(${TARGET_NAME} PRIVATE _WIN32_WINNT=0x0601)
endif()

target_link_libraries(${TARGET_NAME} PRIVATE ${Boost_LIBRARIES})

include(GNUInstallDirs)
install(
    TARGETS ${TARGET_NAME} COMPONENT app
    PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

