
set(JAVASCRIPT_BINDINGS_PLATFORM "WINDOWS")

include(EngineDesktop)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# compile with static runtime
if (MSVC)
    set(ENGINE_MSVC_RUNTIME "/MT")
    msvc_set_runtime(${ENGINE_MSVC_RUNTIME})
else ()
    set (ENGINE_WEBVIEW 0)
endif ()
