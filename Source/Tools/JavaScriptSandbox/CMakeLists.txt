add_executable(JavaScriptSandbox JavaScriptSandbox.cpp)
target_link_libraries(JavaScriptSandbox ${ENGINE_CORE_LIB_TARGET} ${ENGINE_PLUGIN_JS_INTERFACE_TARGET})
target_compile_definitions(JavaScriptSandbox PUBLIC -DENGINE_PLUGIN_JS_TARGET="${ENGINE_PLUGIN_JS_TARGET}")

add_dependencies(JavaScriptSandbox ${ENGINE_PLUGIN_JS_TARGET} ${ENGINE_PLUGIN_JS_SYMLINK_TARGET})

vs_add_to_grp(JavaScriptSandbox "${VS_GRP_ENGINE_TOOLS}")