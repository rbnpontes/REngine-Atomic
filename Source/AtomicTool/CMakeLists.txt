set (ATOMIC_TOOL_SOURCES AtomicTool.cpp AtomicTool.h)

if (WIN32)
    add_definitions(-DATOMIC_WIN32_CONSOLE)
endif(WIN32)

add_executable(AtomicTool ${ATOMIC_TOOL_SOURCES})

target_link_libraries(AtomicTool ToolCore Poco Atomic)

if (WIN32)
    target_link_libraries(AtomicTool Iphlpapi Wldap32)
endif()

if (ATOMIC_JAVASCRIPT OR ATOMIC_DOTNET)
    if(ATOMIC_DOTNET)
        target_link_libraries(AtomicTool AtomicNETScript)
    endif()

    yarn_get_command()
    # Custom target which is always considered out of date, checks for zero size/missing script bindings and generates
    # if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    #     add_custom_target(AtomicToolCheckScripts
    #                     WORKING_DIRECTORY "${ATOMIC_SOURCE_DIR}"
    #                     COMMAND "${CMAKE_COMMAND}" -E make_directory \"${ATOMIC_SOURCE_DIR}/Artifacts/Build/AtomicTool\"
    #                     COMMAND "${CMAKE_COMMAND}" -E copy_if_different \"$<TARGET_FILE:AtomicTool>\" \"${ATOMIC_SOURCE_DIR}/Artifacts/Build/AtomicTool/\"
    #                     COMMAND "${YARN_CMD}" "build" "build:genscripts[false]")
    # else ()
    # endif()
    add_custom_target(AtomicToolCheckScripts
                    WORKING_DIRECTORY "${ATOMIC_SOURCE_DIR}"
                    COMMAND "${CMAKE_COMMAND}" -E make_directory \"${ATOMIC_SOURCE_DIR}/Artifacts/Build/AtomicTool\"
                    COMMAND "${CMAKE_COMMAND}" -E copy_if_different \"$<TARGET_FILE:AtomicTool>\" \"${ATOMIC_SOURCE_DIR}/Artifacts/Build/AtomicTool/\"
                    COMMAND "${YARN_CMD}" "--cwd" "Build" "build" "build:genscripts[false]")

    add_dependencies (AtomicToolCheckScripts AtomicTool)

    # Copy AtomicTool to Artifacts
    add_custom_command( TARGET AtomicTool POST_BUILD
                        COMMAND "${CMAKE_COMMAND}"
                        ARGS -E make_directory \"${ATOMIC_SOURCE_DIR}/Artifacts/Build/AtomicTool\"
                        COMMAND "${CMAKE_COMMAND}"
                        ARGS -E copy_if_different \"$<TARGET_FILE:AtomicTool>\" \"${ATOMIC_SOURCE_DIR}/Artifacts/Build/AtomicTool/\"
                        COMMENT "Copying AtomicTool to Build Artifacts" )

    add_custom_target ( GenerateScriptBindings
                        WORKING_DIRECTORY "${ATOMIC_SOURCE_DIR}"
                        COMMAND "${YARN_CMD}" "--cwd" "Build" "build" "build:genscripts[true];--trace"
                        DEPENDS AtomicTool )

    if (ATOMIC_DOTNET)
        add_custom_target ( GenerateAtomicNET
                            WORKING_DIRECTORY "${ATOMIC_SOURCE_DIR}"
                            COMMAND "${YARN_CMD}" "--cwd" "Build" "build" build:genAtomicNET[desktop,$<$<CONFIG:debug>:"Debug">$<$<CONFIG:release>:"Release">];--trace
                            DEPENDS AtomicTool )
    endif()
endif ()