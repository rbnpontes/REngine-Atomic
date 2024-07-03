if (WIN32)
    include(EngineWindows)
elseif (APPLE)
    if (IOS)
        include(EngineIOS)
    else ()
        include(EngineMac)
    endif ()
elseif (LINUX)
    include(EngineLinux)
elseif (ANDROID)
    include(EngineAndroid)
elseif (WEB)
    include(EngineWeb)
endif ()

message(STATUS "Engine Platform: ${JAVASCRIPT_BINDINGS_PLATFORM}")

set(JAVASCRIPT_BINDINGS_PLATFORM_ROOT "${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated")

# Check if node_modules exists at Build directory
# if not, we must run yarn command to generate then
if(NOT EXISTS "${ENGINE_SOURCE_DIR}/Build/node_modules")
    message(STATUS "[Building]: Installing Dependencies")
    execute_yarn()
    if(NOT YARN_RESULT STREQUAL "0")
        message(FATAL_ERROR "[Building]: Failed to Install Dependencies")
    endif()
endif ()

if (NOT EXISTS "${JAVASCRIPT_BINDINGS_PLATFORM_ROOT}/Javascript" AND ENGINE_JAVASCRIPT)
    set(YARN_ARGS "build" "build:precreateScriptBindings[${JAVASCRIPT_BINDINGS_PLATFORM}]")
    execute_yarn()
endif ()

file(GLOB_RECURSE JAVASCRIPT_BINDINGS_NATIVE_FILENAMES ${JAVASCRIPT_BINDINGS_PLATFORM_ROOT}/*.cpp ${JAVASCRIPT_BINDINGS_PLATFORM_ROOT}/*.h)
