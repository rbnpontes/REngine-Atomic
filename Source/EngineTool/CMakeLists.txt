set (TOOL_SOURCES AtomicTool.cpp AtomicTool.h)

if (WIN32)
    add_definitions(-DENGINE_WIN32_CONSOLE)
endif()

if(ENGINE_DOTNET)
    add_definitions(-DENGINE_DOTNET=1)
endif()

add_executable(${ENGINE_TOOL_TARGET} ${TOOL_SOURCES})

target_link_libraries(${ENGINE_TOOL_TARGET} ToolCore Poco ${ENGINE_CORE_LIB_TARGET})

if (WIN32)
    target_link_libraries(${ENGINE_TOOL_TARGET} Iphlpapi Wldap32)

    # Copy vswhere to Artifacts
    add_custom_command( TARGET ${ENGINE_TOOL_TARGET} POST_BUILD
                        COMMAND "${CMAKE_COMMAND}"
                        ARGS -E make_directory \"${ENGINE_SOURCE_DIR}/Artifacts/Build/${ENGINE_TOOL_TARGET}\"
                        COMMAND "${CMAKE_COMMAND}"
                        ARGS -E copy_if_different \"${ENGINE_SOURCE_DIR}/Build/Windows/vswhere.exe\" \"${ENGINE_SOURCE_DIR}/Artifacts/\"
                        COMMENT "Copying vswhere to Build Artifacts" )
endif()

if (ENGINE_JAVASCRIPT OR ENGINE_DOTNET)
    if(ENGINE_DOTNET)
        target_link_libraries(${ENGINE_TOOL_TARGET} ${ENGINE_NET_SCRIPT_TARGET})
    endif()

    yarn_get_command()

    set (TOOL_CHECK_SCRIPTS ${YARN_CMD} "bindings:all")

    add_custom_target(EngineToolCheckScripts
                    WORKING_DIRECTORY "${ENGINE_SOURCE_DIR}"
                    COMMAND "${CMAKE_COMMAND}" -E make_directory \"${ENGINE_SOURCE_DIR}/Artifacts/Build/${ENGINE_TOOL_TARGET}\"
                    COMMAND "${CMAKE_COMMAND}" -E copy_if_different \"$<TARGET_FILE:${ENGINE_TOOL_TARGET}>\" \"${ENGINE_SOURCE_DIR}/Artifacts/Build/${ENGINE_TOOL_TARGET}/\"
                    COMMAND ${TOOL_CHECK_SCRIPTS})

    add_dependencies (EngineToolCheckScripts ${ENGINE_TOOL_TARGET})

    # Copy Engine Tool to Artifacts
    add_custom_command( TARGET ${ENGINE_TOOL_TARGET} POST_BUILD
                        COMMAND "${CMAKE_COMMAND}"
                        ARGS -E make_directory \"${ENGINE_SOURCE_DIR}/Artifacts/Build/${ENGINE_TOOL_TARGET}\"
                        COMMAND "${CMAKE_COMMAND}"
                        ARGS -E copy_if_different \"$<TARGET_FILE:${ENGINE_TOOL_TARGET}>\" \"${ENGINE_SOURCE_DIR}/Artifacts/Build/${ENGINE_TOOL_TARGET}/\"
                        COMMENT "Copying ${ENGINE_TOOL_TARGET} to Build Artifacts" )

    set(GENERATE_SCRIPTS_BINDINGS ${YARN_CMD} "bindings:gen")
    add_custom_target (GenerateScriptBindings
                        WORKING_DIRECTORY "${ENGINE_SOURCE_DIR}"
                        COMMAND ${GENERATE_SCRIPTS_BINDINGS}
                        DEPENDS ${ENGINE_TOOL_TARGET})

    if (ENGINE_DOTNET)
        set(GENERATE_ENGINE_NET ${YARN_CMD} "build" build:genAtomicNET[desktop,$<$<CONFIG:debug>:"Debug">$<$<CONFIG:release>:"Release">];--trace)
        if(APPLE)
            set(GENERATE_ENGINE_NET ${YARN_CMD} "build build:genAtomicNET[desktop,$<$<CONFIG:debug>:\"Debug\">$<$<CONFIG:release>:\"Release\">];--trace")
        endif()

        add_custom_target (GenerateAtomicNET
                            WORKING_DIRECTORY "${ENGINE_SOURCE_DIR}"
                            COMMAND ${GENERATE_ENGINE_NET}
                            DEPENDS ${ENGINE_TOOL_TARGET})
    endif()
endif ()

vs_add_to_grp(${ENGINE_TOOL_TARGET} "${VS_GRP_ENGINE_TOOLS}")
vs_add_to_grp(EngineToolCheckScripts "${VS_GRP_ENGINE_TOOLS}")
vs_add_to_grp(GenerateScriptBindings "${VS_GRP_ENGINE_TOOLS}")
vs_add_to_grp(GenerateAtomicNET "${VS_GRP_ENGINE_TOOLS}")