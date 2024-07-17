set (CS_NET_SCRIPT_NATIVE_DIR "${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/CSharp/Packages/${ENGINE_NET_SCRIPT_TARGET}/Native")
set (JS_NET_SCRIPT_DIR "${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/Javascript/Packages/${ENGINE_NET_SCRIPT_TARGET}")

file ( GLOB NET_SCRIPT_BINDINGS_SOURCE ${CS_NET_SCRIPT_NATIVE_DIR}/*.cpp ${CS_NET_SCRIPT_NATIVE_DIR}/*.h
                                            ${JS_NET_SCRIPT_DIR}/*.cpp ${JS_NET_SCRIPT_DIR}/*.h )

file (GLOB_RECURSE SOURCE_FILES *.cpp *.h)

add_library(${ENGINE_NET_SCRIPT_TARGET} ${SOURCE_FILES})
target_link_libraries(${ENGINE_NET_SCRIPT_TARGET} ${ENGINE_CORE_LIB_TARGET})
add_library(${ENGINE_NET_SCRIPT_BINDINGS_TARGET} ${NET_SCRIPT_BINDINGS_SOURCE})
target_link_libraries(${ENGINE_NET_SCRIPT_BINDINGS_TARGET} ${ENGINE_CORE_LIB_TARGET})
if (ENGINE_DESKTOP)
    add_dependencies(${ENGINE_NET_SCRIPT_BINDINGS_TARGET} AtomicToolCheckScripts)
endif ()

vs_add_to_grp(${ENGINE_NET_SCRIPT_TARGET} "${VS_GRP_ENGINE_SRC}")
vs_add_to_grp(${ENGINE_NET_SCRIPT_BINDINGS_TARGET} "${VS_GRP_ENGINE_SRC}")