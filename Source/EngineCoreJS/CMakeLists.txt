
file (GLOB JAVASCRIPT_SOURCE Javascript/*.cpp Javascript/*.h)

GroupSources("Javascript")

if (NOT WIN32)
    # for kNet
    add_definitions (-DUNIX)
endif()

string(TOLOWER "${ENGINE_CORE_TARGET}" ENGINE_CORE_LOWER_TARGET)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Javascript/JSPackageCore.cpp.in" 
    "${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/Javascript/JSPackageCore.cpp"
)
file (GLOB JAVASCRIPT_BINDINGS_SOURCE ${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/Javascript/Packages/${ENGINE_CORE_TARGET}/*.cpp)
list(APPEND JAVASCRIPT_BINDINGS_SOURCE "${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/Javascript/JSPackageCore.cpp")

set (SOURCE_FILES ${JAVASCRIPT_SOURCE} ${JAVASCRIPT_BINDINGS_SOURCE})

add_library(${ENGINE_JS_TARGET} ${SOURCE_FILES})
target_link_libraries(${ENGINE_JS_TARGET} ${ENGINE_CORE_LIB_TARGET})
if (ENGINE_DESKTOP)
    add_dependencies(${ENGINE_JS_TARGET} EngineToolCheckScripts)
endif ()

vs_add_to_grp(${ENGINE_JS_TARGET} "${VS_GRP_ENGINE_SRC}")