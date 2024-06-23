if(APPLE)
    if (POLICY CMP0037)
        # new cmake doesn't like creating framework whose name has spaces
        # which CEF3 scripts (including shell) currently require on OSX
        cmake_policy(SET CMP0037 OLD)
    endif ()
    set(CEF_ROOT "${ATOMIC_SOURCE_DIR}/Source/ThirdParty/CEF/MacOSX/${PROJECT_ARCH}")
    set(CEF_PREPARE_PLATFORM "macos-x86-64")
    
    if(PROJECT_ARCH STREQUAL "arm64")
        set(CEF_PREPARE_PLATFORM "macos-arm64")
    endif()
    
    add_subdirectory(MacOSX/${PROJECT_ARCH})
elseif (WIN32)
    set(CEF_ROOT "${ATOMIC_SOURCE_DIR}/Source/ThirdParty/CEF/Windows")
    set(CEF_PREPARE_PLATFORM "windows")
    add_subdirectory(Windows)
else()
    set(CEF_ROOT "${ATOMIC_SOURCE_DIR}/Source/ThirdParty/CEF/Linux")
    set(CEF_PREPARE_PLATFORM "linux")
    add_subdirectory(Linux)
endif()

# Setup CEF binaries
set(YARN_ARGS "cef:prepare:${CEF_PREPARE_PLATFORM}")
execute_yarn()
