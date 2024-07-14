file (GLOB_RECURSE SOURCE_FILES *.cpp *.h)

add_library(${ENGINE_APP_TARGET} ${SOURCE_FILES})
target_link_libraries(${ENGINE_APP_TARGET} ${ENGINE_CORE_LIB_TARGET})
target_compile_definitions(${ENGINE_APP_TARGET} PUBLIC -DENGINE_APP_TARGET="${ENGINE_APP_TARGET}")

if (ENGINE_DESKTOP)
    if(ENGINE_JAVASCRIPT OR ENGINE_DOTNET)
        add_dependencies(${ENGINE_APP_TARGET} AtomicToolCheckScripts)
    endif()
endif ()
if(ENGINE_JAVASCRIPT)
    target_link_libraries(${ENGINE_APP_TARGET} AtomicJS AtomicPlayerJS)
endif()
GroupSources("Player")

vs_add_to_grp(${ENGINE_APP_TARGET} "${VS_GRP_ENGINE_SRC}")