include(EngineVsGroupNames)

add_subdirectory(ThirdParty)
add_subdirectory(EngineCore)

if (ENGINE_JAVASCRIPT)
    add_subdirectory(EnginePlayer)
    add_subdirectory(EngineCoreJS)
    add_subdirectory(EnginePlayerJS)
endif ()

if (ENGINE_WEBVIEW)
    add_subdirectory(EngineWebView)
endif ()

if (ENGINE_JAVASCRIPT OR ENGINE_DOTNET)
    add_subdirectory(EngineApp)
    if (ENGINE_DOTNET)
        add_subdirectory(EngineNET)
    endif()
endif ()

add_subdirectory(Plugins)

if (ENGINE_DESKTOP)
    add_subdirectory(EngineTool)
    add_subdirectory(Tools)
    add_subdirectory(ToolCore)
    if (ENGINE_JAVASCRIPT)
        add_subdirectory(ToolCoreJS)
    endif ()
    if (ENGINE_WEBVIEW AND ENGINE_EDITOR)
        add_subdirectory(EngineEditor)
    endif ()
endif ()

include(EngineDocList)
