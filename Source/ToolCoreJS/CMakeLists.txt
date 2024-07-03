file (GLOB_RECURSE SOURCE_FILES *.cpp *.h)

file (GLOB JAVASCRIPT_BINDINGS_SOURCE ${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/Javascript/Packages/ToolCore/*.cpp)

set (SOURCE_FILES ${SOURCE_FILES} ${JAVASCRIPT_BINDINGS_SOURCE} )

set (CMAKE_CXX_STANDARD 14)
add_library(ToolCoreJS ${SOURCE_FILES})

target_link_libraries(ToolCoreJS ToolCore Assimp)

add_dependencies(ToolCoreJS AtomicToolCheckScripts)

# Put it back C++ version
set (CMAKE_CXX_STANDARD 20)

vs_add_to_grp(ToolCoreJS "${VS_GRP_ENGINE_TOOLS}")