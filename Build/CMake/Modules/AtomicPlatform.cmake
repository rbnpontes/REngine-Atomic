
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

set(RENGINE_GEN_COMMAND ${RENGINE_GEN_COMMAND} build:precreateScriptBindings[${JAVASCRIPT_BINDINGS_PLATFORM}])

message(STATUS "[Building Bindings]: ${RENGINE_GEN_COMMAND}")
if (NOT EXISTS "${JAVASCRIPT_BINDINGS_PLATFORM_ROOT}/Javascript")
    execute_process(COMMAND ${RENGINE_GEN_COMMAND}
        WORKING_DIRECTORY "${ATOMIC_SOURCE_DIR}" 
        RESULTS_VARIABLE RENGINE_PRECREATE_SCRIPT_BINDINGS_RESULT)
    message(STATUS "[Building Precreate]: ${RENGINE_PRECREATE_SCRIPT_BINDINGS_RESULT}")
endif ()

file(GLOB_RECURSE JAVASCRIPT_BINDINGS_NATIVE_FILENAMES ${JAVASCRIPT_BINDINGS_PLATFORM_ROOT}/*.cpp ${JAVASCRIPT_BINDINGS_PLATFORM_ROOT}/*.h)
