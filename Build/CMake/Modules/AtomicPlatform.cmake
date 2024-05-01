
set(ATOMIC_DYNAMIC_RUNTIME OFF CACHE BOOL "Build engine as shared library and link dynamically to system runtime.")

set (RENGINE_GEN_COMMAND "yarn build")

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
    set (RENGINE_GEN_COMMAND ${ATOMIC_SOURCE_DIR}/Build/Scripts/Windows/PrecreateScriptBindings.bat ${ATOMIC_SOURCE_DIR})
endif()

if (WIN32)
    include(AtomicWindows)
elseif (APPLE)
    if (IOS)
        include(AtomicIOS)
    else ()
        include(AtomicMac)
    endif ()
elseif (LINUX)
    include(AtomicLinux)
elseif (ANDROID)
    include(AtomicAndroid)
elseif (WEB)
    include(AtomicWeb)
endif ()

message(STATUS "Atomic platform: ${JAVASCRIPT_BINDINGS_PLATFORM}")

set(JAVASCRIPT_BINDINGS_PLATFORM_ROOT "${ATOMIC_SOURCE_DIR}/Artifacts/Build/Source/Generated")

# Check if node_modules exists at Build directory
# if not, we must run yarn command to generate then
if(NOT EXISTS "${ATOMIC_SOURCE_DIR}/Build/node_modules")
    message(STATUS "[Building]: Installing Dependencies")
    execute_process(COMMAND "yarn"
        WORKING_DIRECTORY "${ATOMIC_SOURCE_DIR}/Build"
        RESULTS_VARIABLE RENGINE_DEPS_INSTALL_RESULT)
    if(NOT RENGINE_DEPS_INSTALL_RESULT STREQUAL "0")
        message(FATAL_ERROR "[Building]: Failed to Install Dependencies")
    endif()
endif ()

if (NOT EXISTS "${JAVASCRIPT_BINDINGS_PLATFORM_ROOT}/Javascript")
    execute_process(COMMAND "yarn" "build" "build:precreateScriptBindings[${JAVASCRIPT_BINDINGS_PLATFORM}]"
        WORKING_DIRECTORY "${ATOMIC_SOURCE_DIR}/Build" 
        RESULTS_VARIABLE RENGINE_PRECREATE_SCRIPT_BINDINGS_RESULT)
    if (NOT RENGINE_PRECREATE_SCRIPT_BINDINGS_RESULT STREQUAL "0")
        message(FATAL_ERROR "Failed to create script bindings")
    endif()
endif ()

file(GLOB_RECURSE JAVASCRIPT_BINDINGS_NATIVE_FILENAMES ${JAVASCRIPT_BINDINGS_PLATFORM_ROOT}/*.cpp ${JAVASCRIPT_BINDINGS_PLATFORM_ROOT}/*.h)
