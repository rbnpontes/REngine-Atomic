
set(JAVASCRIPT_BINDINGS_PLATFORM "WINDOWS")

include(EngineDesktop)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if (MSVC)
    set(ENGINE_MSVC_RUNTIME "/MT")
    msvc_set_runtime(${ENGINE_MSVC_RUNTIME})
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else ()
    set (ENGINE_WEBVIEW 0)
endif ()
