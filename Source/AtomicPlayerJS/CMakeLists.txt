include_directories (${ENGINE_SOURCE_DIR}/Source/ThirdParty)

file (GLOB_RECURSE SOURCE_FILES *.cpp *.h)

file (GLOB JAVASCRIPT_BINDINGS_SOURCE ${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/Javascript/Packages/${ENGINE_PLAYER_TARGET}/*.cpp)

set (SOURCE_FILES ${SOURCE_FILES} ${JAVASCRIPT_BINDINGS_SOURCE} )

add_library(AtomicPlayerJS ${SOURCE_FILES})

target_link_libraries(AtomicPlayerJS ${ENGINE_PLAYER_TARGET_LIB})

if (ENGINE_DESKTOP)
    add_dependencies(AtomicPlayerJS AtomicToolCheckScripts)
endif ()

vs_add_to_grp(AtomicPlayerJS "${VS_GRP_ENGINE_SRC}")