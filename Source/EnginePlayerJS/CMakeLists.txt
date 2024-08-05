include_directories (${ENGINE_SOURCE_DIR}/Source/ThirdParty)

file (GLOB_RECURSE SOURCE_FILES *.cpp *.h)

file (GLOB JAVASCRIPT_BINDINGS_SOURCE ${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/Javascript/Packages/${ENGINE_PLAYER_TARGET}/*.cpp)

set (SOURCE_FILES ${SOURCE_FILES} ${JAVASCRIPT_BINDINGS_SOURCE} )

add_library(${ENGINE_PLAYER_JS_TARGET} ${SOURCE_FILES})

target_link_libraries(${ENGINE_PLAYER_JS_TARGET} ${ENGINE_PLAYER_TARGET_LIB})

if (ENGINE_DESKTOP)
    add_dependencies(${ENGINE_PLAYER_JS_TARGET} EngineToolCheckScripts)
endif ()

vs_add_to_grp(${ENGINE_PLAYER_JS_TARGET} "${VS_GRP_ENGINE_SRC}")