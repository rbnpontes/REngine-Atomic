file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

# Remove the web helper sources
list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/WebView/WebProcessHelperMac.cpp)
file(GLOB JAVASCRIPT_BINDINGS_SOURCE ${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/Javascript/Packages/Editor/*.cpp)
set(SOURCE_FILES ${SOURCE_FILES} ${JAVASCRIPT_BINDINGS_SOURCE})
set(SOURCE_FILES ${SOURCE_FILES} ${CSHARP_BINDINGS_SOURCE} ${CSHARPATOMICPLAYER_BINDINGS_SOURCE} ${CSHARPATOMICNET_BINDINGS_SOURCE})

if (APPLE)
    file(GLOB_RECURSE OBJC_FILES *.mm *.h)
    set(SOURCE_FILES ${SOURCE_FILES} ${OBJC_FILES})
    set(EXE_TYPE MACOSX_BUNDLE)
    #ICNS
    set(MACOSX_BUNDLE_ICON_FILE EngineEditor.icns)
    set(ENGINE_EDITOR_ICON ${ENGINE_SOURCE_DIR}/Build/CMake/Modules/EngineEditor.icns)
    set_source_files_properties(${ENGINE_EDITOR_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
else ()
    # We want a console for development builds
    if (ENGINE_DEV_BUILD AND WIN32)
        add_definitions(-DENGINE_WIN32_CONSOLE)
    else ()
        set(EXE_TYPE WIN32)
    endif ()
    set(SOURCE_FILES ${SOURCE_FILES} ${ENGINE_SOURCE_DIR}/Build/CMake/Modules/EngineEditor.rc)
endif ()

if (ENGINE_WEBVIEW)
    # Target executable names.
    set(CEF_TARGET "${ENGINE_EDITOR_TARGET}")
    if (OS_MACOSX)
        set(CEF_HELPER_TARGET "${ENGINE_EDITOR_TARGET}-Helper")
    endif ()

    # Logical target used to link the libcef library.
    ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

    # Determine the target output directory.
    SET_CEF_TARGET_OUT_DIR()

    #
    # Windows configuration.
    #
    if (OS_WINDOWS)
        # Executable target.
        add_executable(${CEF_TARGET} ${EXE_TYPE} ${SOURCE_FILES})
        add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
        SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
        target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})

        if (USE_SANDBOX)
            # Logical target used to link the cef_sandbox library.
            ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
            target_link_libraries(${CEF_TARGET} cef_sandbox_lib ${CEF_SANDBOX_STANDARD_LIBS})
        endif ()

        # Add the custom manifest files to the executable.
        ADD_WINDOWS_MANIFEST("${CMAKE_CURRENT_SOURCE_DIR}/Windows" "${CEF_TARGET}" "exe")

        # Copy binary and resource files to the target output directory.
        COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
        COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
    endif ()

    #
    # Mac OS X configuration.
    #
    if (APPLE)
        # Output paths for the app bundles.
        set(CEF_APP "${CEF_TARGET_OUT_DIR}/${CEF_TARGET}.app")
        set(CEF_HELPER_APP "${CEF_TARGET_OUT_DIR}/${CEF_HELPER_TARGET}.app")

        # Variable referenced from Info.plist files.
        set(PRODUCT_NAME "${CEF_TARGET}")

        # Helper executable target.
        add_executable(${CEF_HELPER_TARGET} MACOSX_BUNDLE "${CMAKE_CURRENT_SOURCE_DIR}/WebView/WebProcessHelperMac.cpp")
        SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_HELPER_TARGET})
        add_dependencies(${CEF_HELPER_TARGET} libcef_dll_wrapper)
        target_link_libraries(${CEF_HELPER_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS} ${ENGINE_CORE_LIB_TARGET} AtomicWebView)

        set_target_properties(${CEF_HELPER_TARGET} PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${ENGINE_SOURCE_DIR}/Build/CMake/Modules/EngineEditor-HelperInfo.plist.template"
        )

        # Main executable target.
        add_executable(${CEF_TARGET} ${EXE_TYPE} ${SOURCE_FILES} ${ENGINE_EDITOR_ICON})

        SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
        add_dependencies(${CEF_TARGET} libcef_dll_wrapper "${CEF_HELPER_TARGET}")
        target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})
        set_target_properties(${CEF_TARGET} PROPERTIES
            RESOURCE "${CEFSIMPLE_RESOURCES_SRCS}"
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist
        )

        # Copy files into the main app bundle.
        add_custom_command(
            TARGET ${CEF_TARGET}
            POST_BUILD
            # Copy the helper app bundle into the Frameworks directory.
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_HELPER_APP}"
            "${CEF_APP}/Contents/Frameworks/${CEF_HELPER_TARGET}.app"
            # Copy the CEF framework into the Frameworks directory.
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
            "${CEF_APP}/Contents/Frameworks/CEF.framework"
            VERBATIM
        )

        if (NOT ${CMAKE_GENERATOR} STREQUAL "Xcode")
            # Manually process and copy over resource files.
            # The Xcode generator handles this via the set_target_properties RESOURCE directive.
            set(PREFIXES "mac/")  # Remove these prefixes from input file paths.
            COPY_MACOSX_RESOURCES("${CEFSIMPLE_RESOURCES_SRCS}" "${PREFIXES}" "${CEF_TARGET}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_APP}")
        endif ()
    endif ()

    if (OS_LINUX)
        # Executable target.
        add_executable(${CEF_TARGET} ${EXE_TYPE} ${SOURCE_FILES} ${ENGINE_EDITOR_ICON})
        SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
        add_dependencies(${CEF_TARGET} libcef_dll_wrapper)

        # Set rpath so that libraries can be placed next to the executable.
        set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
        set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
        set_target_properties(${CEF_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR})

        # Copy binary and resource files to the target output directory.
        COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
        COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
    endif ()

    target_compile_definitions(${CEF_TARGET} PUBLIC -DENGINE_EDITOR_NAME="${ENGINE_EDITOR_NAME}")
endif ()

target_link_libraries(${CEF_TARGET} ToolCore AtomicWebView AtomicJS AtomicPlayerJS ToolCoreJS Poco nativefiledialog ${ENGINE_CORE_LIB_TARGET} EASTL)

if(ENGINE_JAVASCRIPT OR ENGINE_DOTNET)
    target_link_libraries(${CEF_TARGET} AtomicApp)
endif()

if(ENGINE_DOTNET)
    target_compile_definitions(${CEF_TARGET} PUBLIC -DENGINE_DOTNET=1)
    target_link_libraries(${CEF_TARGET} AtomicNETScriptBindings AtomicNETScript)
endif()

if (APPLE)
    set(TARGET_PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${ENGINE_SOURCE_DIR}/Build/CMake/Modules/EngineEditorInfo.plist.template")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.rengine.editor")
    set(MACOSX_BUNDLE_BUNDLE_NAME "${ENGINE_EDITOR_TARGET}")
    target_link_libraries(${ENGINE_EDITOR_TARGET})
elseif (LINUX)
    target_link_libraries(${ENGINE_EDITOR_TARGET} nativefiledialog ${GTK3_LIBRARIES})
    target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})
else ()
    target_link_libraries(${ENGINE_EDITOR_TARGET} Iphlpapi Wldap32)
endif ()

if (TARGET_PROPERTIES)
    set_target_properties(${ENGINE_EDITOR_TARGET} PROPERTIES ${TARGET_PROPERTIES})
endif ()

target_link_libraries(${ENGINE_EDITOR_TARGET} ${ENGINE_CORE_LIB_TARGET})

add_dependencies(${CEF_TARGET} AtomicToolCheckScripts)

GroupSources("Application")
GroupSources("EditorMode")
GroupSources("Editors")
GroupSources("Javascript")
GroupSources("PlayerMode")
GroupSources("Utils")
GroupSources("Components")

include(EngineDoc)

vs_add_to_grp("${CEF_TARGET}" "${VS_GRP_ENGINE_TOOLS}")