add_subdirectory(PackageTool)
add_subdirectory(JavaScriptSandbox)

if (WIN32)
    include_external_msproject(
        BindingGenerator ${CMAKE_CURRENT_LIST_DIR}/BindingGenerator/BindingGenerator.csproj)
    vs_add_to_grp(BindingGenerator "${VS_GRP_ENGINE_TOOLS}")
endif()
