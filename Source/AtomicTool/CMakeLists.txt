set (ATOMIC_TOOL_SOURCES AtomicTool.cpp AtomicTool.h)

if (WIN32)
    add_definitions(-DENGINE_WIN32_CONSOLE)
endif()

if(ENGINE_DOTNET)
    add_definitions(-DENGINE_DOTNET=1)
endif()

add_executable(AtomicTool ${ATOMIC_TOOL_SOURCES})

target_link_libraries(AtomicTool ToolCore Poco ${ENGINE_CORE_LIB_TARGET})

if (WIN32)
    target_link_libraries(AtomicTool Iphlpapi Wldap32)

    # Copy vswhere to Artifacts
    add_custom_command( TARGET AtomicTool POST_BUILD
                        COMMAND "${CMAKE_COMMAND}"
                        ARGS -E make_directory \"${ENGINE_SOURCE_DIR}/Artifacts/Build/AtomicTool\"
                        COMMAND "${CMAKE_COMMAND}"
                        ARGS -E copy_if_different \"${ENGINE_SOURCE_DIR}/Build/Windows/vswhere.exe\" \"${ENGINE_SOURCE_DIR}/Artifacts/\"
                        COMMENT "Copying vswhere to Build Artifacts" )
endif()

if (ENGINE_JAVASCRIPT OR ENGINE_DOTNET)
    if(ENGINE_DOTNET)
        target_link_libraries(AtomicTool ${ENGINE_NET_SCRIPT_TARGET})
    endif()

    yarn_get_command()

    set (TOOL_CHECK_SCRIPTS ${YARN_CMD} "build" "build:genscripts[false]")
    if(APPLE)
        set(TOOL_CHECK_SCRIPTS ${YARN_CMD} "build build:genscripts[false]")
    endif()

    add_custom_target(AtomicToolCheckScripts
                    WORKING_DIRECTORY "${ENGINE_SOURCE_DIR}"
                    COMMAND "${CMAKE_COMMAND}" -E make_directory \"${ENGINE_SOURCE_DIR}/Artifacts/Build/AtomicTool\"
                    COMMAND "${CMAKE_COMMAND}" -E copy_if_different \"$<TARGET_FILE:AtomicTool>\" \"${ENGINE_SOURCE_DIR}/Artifacts/Build/AtomicTool/\"
                    COMMAND ${TOOL_CHECK_SCRIPTS})

    add_dependencies (AtomicToolCheckScripts AtomicTool)

    # Copy AtomicTool to Artifacts
    add_custom_command( TARGET AtomicTool POST_BUILD
                        COMMAND "${CMAKE_COMMAND}"
                        ARGS -E make_directory \"${ENGINE_SOURCE_DIR}/Artifacts/Build/AtomicTool\"
                        COMMAND "${CMAKE_COMMAND}"
                        ARGS -E copy_if_different \"$<TARGET_FILE:AtomicTool>\" \"${ENGINE_SOURCE_DIR}/Artifacts/Build/AtomicTool/\"
                        COMMENT "Copying AtomicTool to Build Artifacts" )

    set(GENERATE_SCRIPTS_BINDINGS ${YARN_CMD} "build" "build:genscripts[true];--trace")
    if(APPLE)
        set(GENERATE_SCRIPTS_BINDINGS ${YARN_CMD} "build build:genscripts[true];--trace")
    endif()

    add_custom_target ( GenerateScriptBindings
                        WORKING_DIRECTORY "${ENGINE_SOURCE_DIR}"
                        COMMAND ${GENERATE_SCRIPTS_BINDINGS}
                        DEPENDS AtomicTool )

    if (ENGINE_DOTNET)
        set(GENERATE_ENGINE_NET ${YARN_CMD} "build" build:genAtomicNET[desktop,$<$<CONFIG:debug>:"Debug">$<$<CONFIG:release>:"Release">];--trace)
        if(APPLE)
            set(GENERATE_ENGINE_NET ${YARN_CMD} "build build:genAtomicNET[desktop,$<$<CONFIG:debug>:\"Debug\">$<$<CONFIG:release>:\"Release\">];--trace")
        endif()

        add_custom_target ( GenerateAtomicNET
                            WORKING_DIRECTORY "${ENGINE_SOURCE_DIR}"
                            COMMAND ${GENERATE_ENGINE_NET}
                            DEPENDS AtomicTool )
    endif()
endif ()

vs_add_to_grp(AtomicTool "${VS_GRP_ENGINE_TOOLS}")
vs_add_to_grp(AtomicToolCheckScripts "${VS_GRP_ENGINE_TOOLS}")
vs_add_to_grp(GenerateScriptBindings "${VS_GRP_ENGINE_TOOLS}")
vs_add_to_grp(GenerateAtomicNET "${VS_GRP_ENGINE_TOOLS}")