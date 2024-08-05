set (ENGINE_DESKTOP TRUE)
set (ENGINE_WEBVIEW ON)
add_definitions(-DENGINE_WEBVIEW)

if(APPLE)
    set(CEF_PREPARE_PLATFORM "macos-x86-64")
        
    if(PROJECT_ARCH STREQUAL "arm64")
        set(CEF_PREPARE_PLATFORM "macos-arm64")
    endif()
elseif(WIN32)
    set(CEF_PREPARE_PLATFORM "windows")
else()
    set(CEF_PREPARE_PLATFORM "linux")
endif()

# Setup CEF binaries
set(YARN_ARGS "cef:prepare:${CEF_PREPARE_PLATFORM}")
execute_yarn()

include(${ENGINE_SOURCE_DIR}/Source/ThirdParty/CEF/CEF.cmake)
PRINT_CEF_CONFIG()