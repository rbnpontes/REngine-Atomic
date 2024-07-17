add_definitions(-DCPLUSPLUS_WITHOUT_QT)

file (GLOB_RECURSE SOURCE_FILES *.cpp *.h)

if (APPLE)
    file (GLOB_RECURSE OBJC_FILES *.mm )
    set (SOURCE_FILES ${SOURCE_FILES} ${OBJC_FILES})
endif()

set(CMAKE_CXX_STANDARD 20)
add_library(ToolCore ${SOURCE_FILES})

target_link_libraries(ToolCore Assimp Poco ${ENGINE_CORE_LIB_TARGET})

if (ENGINE_XCODE)
    target_compile_definitions(ToolCore PRIVATE -DENGINE_XCODE)
endif ()

target_compile_definitions(ToolCore PUBLIC 
    -DENGINE_NAME="${ENGINE_NAME}" 
    -DENGINE_EDITOR_NAME="${ENGINE_EDITOR_NAME}"
    -DENGINE_NET_NAME="${ENGINE_NET_NAME}"
    -DENGINE_NET_NATIVE_TARGET="${ENGINE_NET_NATIVE_TARGET}"
    -DENGINE_NET_SERVICE_NAME="${ENGINE_NET_SERVICE_NAME}"
    -DENGINE_PLAYER_TARGET="${ENGINE_PLAYER_TARGET}"
    -DENGINE_PROJECT_NAME="${ENGINE_PROJECT_NAME}")

GroupSources("Assets")
GroupSources("Build")
GroupSources("Command")
GroupSources("Import")
GroupSources("JSBind")
GroupSources("License")
GroupSources("Net")
GroupSources("Platform")
GroupSources("Project")
GroupSources("Subprocess")
GroupSources("NETTools")

vs_add_to_grp(ToolCore "${VS_GRP_ENGINE_TOOLS}")
