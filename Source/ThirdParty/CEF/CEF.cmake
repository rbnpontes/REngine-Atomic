if(APPLE)
    if (POLICY CMP0037)
        # new cmake doesn't like creating framework whose name has spaces
        # which CEF3 scripts (including shell) currently require on OSX
        cmake_policy(SET CMP0037 OLD)
    endif ()
    set(CEF_ROOT "${ATOMIC_SOURCE_DIR}/Source/ThirdParty/CEF/MacOSX/${PROJECT_ARCH}")
elseif (WIN32)
    set(CEF_ROOT "${ATOMIC_SOURCE_DIR}/Source/ThirdParty/CEF/Windows")
else()
    set(CEF_ROOT "${ATOMIC_SOURCE_DIR}/Source/ThirdParty/CEF/Linux")
endif()

message(STATUS "CEF_ROOT: ${CEF_ROOT}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")
find_package(CEF REQUIRED)
include_directories(${CEF_ROOT})