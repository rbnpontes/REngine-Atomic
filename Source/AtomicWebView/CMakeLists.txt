
file (GLOB SOURCE_FILES *.cpp *.h)
file (GLOB INTERNAL_SOURCE_FILES Internal/*.cpp Internal/*.h)

file (GLOB JAVASCRIPT_BINDINGS_SOURCE ${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/Javascript/Packages/WebView/*.cpp)

if (APPLE)
    set (PLATFORM_SOURCE WebBrowserHost.mm)
endif()

set (SOURCE_FILES ${SOURCE_FILES} ${JAVASCRIPT_BINDINGS_SOURCE} ${PLATFORM_SOURCE} ${INTERNAL_SOURCE_FILES})

GroupSources("Internal")

add_library(AtomicWebView ${SOURCE_FILES})
target_link_libraries(AtomicWebView ${ENGINE_CORE_LIB_TARGET})

if (ENGINE_JAVASCRIPT OR ENGINE_DOTNET)
    add_dependencies(AtomicWebView AtomicToolCheckScripts)
endif()

vs_add_to_grp(AtomicWebView "${VS_GRP_ENGINE_SRC}")