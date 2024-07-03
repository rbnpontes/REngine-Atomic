
file (GLOB JAVASCRIPT_SOURCE Javascript/*.cpp Javascript/*.h)

GroupSources("Javascript")

if (NOT WIN32)
    # for kNet
    add_definitions (-DUNIX)
endif()

file (GLOB JAVASCRIPT_BINDINGS_SOURCE ${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/Javascript/Packages/Atomic/*.cpp)

set (SOURCE_FILES ${JAVASCRIPT_SOURCE} ${JAVASCRIPT_BINDINGS_SOURCE})

add_library(AtomicJS ${SOURCE_FILES})
target_link_libraries(AtomicJS ${ENGINE_CORE_LIB_TARGET})
if (ENGINE_DESKTOP)
    add_dependencies(AtomicJS AtomicToolCheckScripts)
endif ()

vs_add_to_grp(AtomicJS "${VS_GRP_ENGINE_SRC}")