set (BINDINGS_JS_BASE_PATH ${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/JS)
set (ENGINE_CORE_BINDING_SRC ${BINDINGS_JS_BASE_PATH}/EngineCore)

file (GLOB_RECURSE SOURCE_FILES ${ENGINE_CORE_BINDING_SRC}/**/*.cpp ${ENGINE_CORE_BINDING_SRC}/**/*.h)
add_library(EngineCoreModule SHARED ${SOURCE_FILES})

target_link_libraries(EngineCoreModule PRIVATE ${ENGINE_CORE_LIB_TARGET} ${ENGINE_PLUGIN_JS_INTERFACE_TARGET})
vs_add_to_grp(EngineCoreModule "${VS_GRP_BINDINGS}")