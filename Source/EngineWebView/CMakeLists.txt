
file (GLOB SOURCE_FILES *.cpp *.h)
file (GLOB INTERNAL_SOURCE_FILES Internal/*.cpp Internal/*.h)

file (GLOB JAVASCRIPT_BINDINGS_SOURCE ${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/Javascript/Packages/WebView/*.cpp)

if (APPLE)
    set (PLATFORM_SOURCE WebBrowserHost.mm)
endif()

set (SOURCE_FILES ${SOURCE_FILES} ${JAVASCRIPT_BINDINGS_SOURCE} ${PLATFORM_SOURCE} ${INTERNAL_SOURCE_FILES})

GroupSources("Internal")

add_library(${ENGINE_WEB_VIEW_TARGET} ${SOURCE_FILES})
target_link_libraries(${ENGINE_WEB_VIEW_TARGET} ${ENGINE_CORE_LIB_TARGET})

if (ENGINE_JAVASCRIPT OR ENGINE_DOTNET)
    add_dependencies(${ENGINE_WEB_VIEW_TARGET} AtomicToolCheckScripts)
endif()

vs_add_to_grp(${ENGINE_WEB_VIEW_TARGET} "${VS_GRP_ENGINE_SRC}")