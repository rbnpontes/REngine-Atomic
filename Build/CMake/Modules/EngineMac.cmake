set(JAVASCRIPT_BINDINGS_PLATFORM "MACOSX")

# for CEF3
set(PROJECT_ARCH "x86_64")

set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(CMAKE_OSX_ARCHITECTURES arm64)
    set(PROJECT_ARCH "arm64")
endif()

include(EngineDesktop)

if (CMAKE_GENERATOR STREQUAL "Xcode")
    set(ENGINE_XCODE 1)
elseif (NOT CMAKE_CROSSCOMPILING)
    # When not using XCode, linker takes a long time, which this flag seems to be being passed during xcode builds
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker -no_deduplicate")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Xlinker -no_deduplicate")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof -stdlib=libc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-command-line-argument -Wno-unused-command-line-argument")

link_directories(/usr/local/lib)
