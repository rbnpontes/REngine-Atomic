
set(ATOMIC_DYNAMIC_RUNTIME OFF CACHE BOOL "Build engine as shared library and link dynamically to system runtime.")

set (RENGINE_NODE_JAKE yarn -cwd ${ATOMIC_SOURCE_DIR}/Build build)

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

# if (NOT WIN32)
#     set(ATOMIC_OPENGL 1)
# endif ()

message(STATUS "Atomic platform: ${JAVASCRIPT_BINDINGS_PLATFORM}")

set(JAVASCRIPT_BINDINGS_PLATFORM_ROOT "${ATOMIC_SOURCE_DIR}/Artifacts/Build/Source/Generated")

message(STATUS "JAVASCRIPT_BINDINGS_PLATFORM_ROOT: ${JAVASCRIPT_BINDINGS_PLATFORM_ROOT}")
if (NOT EXISTS "${JAVASCRIPT_BINDINGS_PLATFORM_ROOT}/Javascript")
    execute_process(COMMAND ${RENGINE_NODE_JAKE} --task=build:precreateScriptBindings[${JAVASCRIPT_BINDINGS_PLATFORM}]
        WORKING_DIRECTORY "${ATOMIC_SOURCE_DIR}" RESULTS_VARIABLE RENGINE_PRECREATE_SCRIPT_BINDINGS_RESULT)
    message(STATUS "[Building Precreate]: ${RENGINE_PRECREATE_SCRIPT_BINDINGS_RESULT}")
endif ()

file(GLOB_RECURSE JAVASCRIPT_BINDINGS_NATIVE_FILENAMES ${JAVASCRIPT_BINDINGS_PLATFORM_ROOT}/*.cpp ${JAVASCRIPT_BINDINGS_PLATFORM_ROOT}/*.h)
