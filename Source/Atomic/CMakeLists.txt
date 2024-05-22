file (GLOB CONTAINER_SOURCE Container/*.cpp Container/*.h)
file (GLOB CORE_SOURCE Core/*.cpp Core/*.h)
file (GLOB MATH_SOURCE Math/*.cpp Math/*.h)
file (GLOB IO_SOURCE IO/*.cpp IO/*.h)
file (GLOB ENGINE_SOURCE Engine/*.cpp Engine/*.h)
file (GLOB INPUT_SOURCE Input/*.cpp Input/*.h)
file (GLOB RESOURCE_SOURCE Resource/*.cpp Resource/*.h)
file (GLOB AUDIO_SOURCE Audio/*.cpp Audio/*.h)
file (GLOB NETWORK_SOURCE Network/*.cpp Network/*.h)
file (GLOB SCRIPT_SOURCE Script/*.cpp Script/*.h)
file (GLOB METRICS_SOURCE Metrics/*.cpp Metrics/*.h)
if (NOT WEB AND NOT IOS AND NOT ANDROID)
    file (GLOB IPC_SOURCE IPC/*.cpp IPC/*.h)
endif()
file (GLOB ATOMIC2D_SOURCE Atomic2D/*.cpp Atomic2D/*.h)
file (GLOB SCENE_SOURCE Scene/*.cpp Scene/*.h)
file (GLOB UI_SOURCE UI/*.cpp UI/*.h)
file (GLOB SYSTEM_UI_SOURCE UI/SystemUI/*.cpp UI/SystemUI/*.h)
file (GLOB PHYSICS_SOURCE Physics/*.cpp Physics/*.h)
file (GLOB NAVIGATION_SOURCE Navigation/*.cpp Navigation/*.h)
file (GLOB ENVIRONMENT_SOURCE Environment/*.cpp Environment/*.h)
file (GLOB GRAPHICS_SOURCE Graphics/*.cpp Graphics/*.h Graphics/Text3D/*.cpp Graphics/Text3D/*.h)
file (GLOB RHI_SOURCE RHI/*.cpp RHI/*.h)

if (ATOMIC_IK)
    file (GLOB IK_SOURCE IK/*.cpp IK/*.h)
endif ()

if (ATOMIC_DATABASE_SQLITE)
    file (GLOB DATABASE_SOURCE Database/*.cpp Database/*.h Database/SQLite/*.cpp Database/SQLite/*.h)
elseif (ATOMIC_DATABASE_ODBC)
    file (GLOB DATABASE_SOURCE Database/*.cpp Database/*.h Database/ODBC/*.cpp Database/ODBC/*.h)
endif ()

if (APPLE AND NOT IOS)
    set (PLATFORM_SOURCE IO/MacFileWatcher.mm UI/UIDragDropMac.mm)
endif()

if(ANDROID)
    file(GLOB PLATFORM_SOURCE IO/Android/*.cpp IO/Android/*.h)
endif()

set (SOURCE_FILES ${CONTAINER_SOURCE} ${CORE_SOURCE} ${ENGINE_SOURCE} ${INPUT_SOURCE}
                  ${AUDIO_SOURCE} ${IO_SOURCE} ${MATH_SOURCE}
                  ${RESOURCE_SOURCE} ${PHYSICS_SOURCE} ${IPC_SOURCE}
                  ${GRAPHICS_SOURCE} ${GRAPHICS_IMPL_SOURCE}
                  ${ATOMIC3D_SOURCE}
                  ${ATOMIC2D_SOURCE} ${ENVIRONMENT_SOURCE}
                  ${SCENE_SOURCE} ${UI_SOURCE} ${SYSTEM_UI_SOURCE}
                  ${SCRIPT_SOURCE} ${METRICS_SOURCE}
                  ${PLATFORM_SOURCE} ${DATABASE_SOURCE} ${IK_SOURCE} 
                  ${NAVIGATION_SOURCE} ${RHI_SOURCE})

if (NOT WEB)
  set (SOURCE_FILES ${SOURCE_FILES} ${NETWORK_SOURCE})
endif()

GroupSources("Atomic2D")
GroupSources("Atomic3D")
GroupSources("Audio")
GroupSources("Container")
GroupSources("Core")
GroupSources("Engine")
GroupSources("Environment")
GroupSources("Graphics")
GroupSources("Input")
GroupSources("IO")
GroupSources("IPC")
GroupSources("Math")
GroupSources("Navigation")
GroupSources("Network")
GroupSources("Physics")
GroupSources("Resource")
GroupSources("Scene")
GroupSources("UI")
GroupSources("Web")
GroupSources("Script")
GroupSources("Metrics")
GroupSources("BuildInfo")
GroupSources("IK")
GroupSources("RHI")

# Handle Git Revision
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/BuildInfo/AtomicGitSHA.cpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/BuildInfo/AtomicGitSHA.cpp" @ONLY)

set (SOURCE_FILES ${SOURCE_FILES} BuildInfo/AtomicBuildInfo.cpp BuildInfo/AtomicBuildInfo.h BuildInfo/AtomicGitSHA.cpp BuildInfo/AtomicGitSHA.h)

if (RENGINE_SHARED)
    set(ATOMIC_LIBRARY_TYPE SHARED)
else ()
    set(ATOMIC_LIBRARY_TYPE STATIC)
endif ()

add_library(Atomic ${ATOMIC_LIBRARY_TYPE} ${SOURCE_FILES})
if (NOT RENGINE_SHARED)
    target_compile_definitions (Atomic PUBLIC -DATOMIC_STATIC_DEFINE=1 -DATOMIC_API=)
else ()
    target_compile_definitions (Atomic 
        PRIVATE -DATOMIC_EXPORTS=1 
    )
endif ()

if(ATOMIC_SSE OR RENGINE_SSE)
    target_compile_definitions(Atomic PUBLIC -DATOMIC_SSE=1 -DRENGINE_SSE=1)
endif ()

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Atomic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)

set (PUBLIC_THIRD_PARTY_DEPS 
    Box2D 
    Duktape 
    TurboBadger 
    FreeType 
    JO 
    LZ4 
    PugiXml
    STB 
    rapidjson 
    imgui
    EASTL)
set (PRIVATE_THIRD_PARTY_DEPS SDL2::SDL2-static)

target_compile_definitions (Atomic PUBLIC -DATOMIC_CXX11=1)
if (ATOMIC_64BIT)
    target_compile_definitions (Atomic PUBLIC -DATOMIC_64BIT=1)
endif ()

if (ANDROID)
    list(APPEND PRIVATE_THIRD_PARTY_DEPS dl log android GLESv1_CM GLESv2)
    target_compile_definitions(Atomic PUBLIC -DATOMIC_PLATFORM_ANDROID=1 -DRENGINE_PLATFORM_ANDROID=1)
elseif (APPLE)
    target_compile_definitions(Atomic PUBLIC -DRENGINE_PLATFORM_APPLE=1)
    list(APPEND PRIVATE_THIRD_PARTY_DEPS
            "-framework AudioToolbox"
            "-framework CoreAudio" 
            "-framework CoreVideo" 
            "-framework IOKit" 
            "-framework CoreServices"
            "-framework Security"
            "-framework SystemConfiguration")
    if (IOS)
        target_compile_definitions(Atomic PUBLIC -DATOMIC_PLATFORM_IOS=1 -DIOS=1 -DRENGINE_PLATFORM_IOS=1)
    else () # MacOS
        list(APPEND PRIVATE_THIRD_PARTY_DEPS 
            "-framework Cocoa"
            "-framework Carbon"
            "-framework ForceFeedback"
            "-framework OpenGL")
        target_compile_definitions(Atomic PUBLIC -DATOMIC_PLATFORM_OSX=1 -DRENGINE_PLATFORM_MACOS=1)
    endif ()
elseif (LINUX)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS pthread dl)
    target_compile_definitions(Atomic PUBLIC -DATOMIC_PLATFORM_LINUX=1)
elseif (WIN32)
    target_compile_definitions (Atomic PUBLIC -DATOMIC_PLATFORM_WINDOWS=1)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid Ws2_32)
elseif (WEB)
    target_compile_definitions(Atomic PUBLIC -DATOMIC_PLATFORM_WEB=1 -DNO_POPEN=1)
endif ()

# Diligent Dependencies
list(APPEND PUBLIC_THIRD_PARTY_DEPS
    Diligent-Common
    Diligent-GraphicsEngineOpenGL-static 
    Diligent-GraphicsEngineVk-static)

list(APPEND PRIVATE_THIRD_PARTY_DEPS glslang
    SPIRV
    spirv-cross-core
    spirv-cross-glsl
    spirv-cross-hlsl
    spirv-cross-reflect
    SPIRV-Tools
    SPIRV-Tools-opt
)

if (WIN32)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS Diligent-GraphicsEngineD3D11-static Diligent-GraphicsEngineD3D12-static)
endif ()

if (ATOMIC_DESKTOP)
    target_compile_definitions (Atomic PUBLIC -DATOMIC_PLATFORM_DESKTOP=1)
    list(APPEND PRIVATE_THIRD_PARTY_DEPS libsquish)
    if (NOT LINUX)
        list(APPEND PRIVATE_THIRD_PARTY_DEPS LibCpuId)
    endif()
    option (ATOMIC_FILEWATCHER "Enable file watcher" ON)
    if (ATOMIC_FILEWATCHER)
        target_compile_definitions(Atomic PUBLIC -DATOMIC_FILEWATCHER=1)
    endif ()
endif ()

if (NOT WEB)
    target_compile_definitions (Atomic PUBLIC -DATOMIC_THREADING=1 -DATOMIC_NETWORK=1 -DATOMIC_WEB=1)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS Civetweb kNet)
endif()

if (ATOMIC_PROFILING)
    target_compile_definitions(Atomic PUBLIC -DATOMIC_PROFILING=1 -DRENGINE_PROFILER_TRACY=1)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS TracyClient)
endif ()

option (ATOMIC_LOGGING "Enable logging" ON)
if (ATOMIC_LOGGING)
    target_compile_definitions (Atomic PUBLIC -DATOMIC_LOGGING=1)
endif ()

option (ATOMIC_2D_ONLY "Build only with 2D support" OFF)
if (ATOMIC_2D_ONLY)
    target_compile_definitions (Atomic PUBLIC -DATOMIC_ATOMIC2D=1)
else ()
    target_compile_definitions (Atomic PUBLIC -DATOMIC_3D)

    option (ATOMIC_ATOMIC2D "Enable 2D subsystem" ON)
    if (ATOMIC_ATOMIC2D)
        target_compile_definitions (Atomic PUBLIC -DATOMIC_ATOMIC2D=1)
    endif ()

    option (ATOMIC_PHYSICS "Enable physics subsystem" ON)
    if (ATOMIC_PHYSICS)
        target_compile_definitions (Atomic PUBLIC -DATOMIC_PHYSICS=1)
        list(APPEND PUBLIC_THIRD_PARTY_DEPS Bullet StanHull)
    endif ()

    option (ATOMIC_NAVIGATION "Enable navigation subsystem" ON)
    if (ATOMIC_NAVIGATION)
        target_compile_definitions (Atomic PUBLIC -DATOMIC_NAVIGATION=1)
        list(APPEND PUBLIC_THIRD_PARTY_DEPS Recast Detour DetourCrowd DetourTileCache)
    endif ()
endif ()

# TODO: enable shared library builds.
if (NOT APPLE OR IOS OR WEB)
    target_compile_definitions (Atomic PUBLIC -DATOMIC_TBUI=1)
endif ()

if (ATOMIC_DATABASE_SQLITE)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS sqlite)
    target_compile_definitions(Atomic PUBLIC -DATOMIC_DATABASE=1 -DATOMIC_DATABASE_SQLITE=1)
elseif (ATOMIC_DATABASE_ODBC)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS nanodbc)
    target_compile_definitions(Atomic PUBLIC -DATOMIC_DATABASE=1 -DATOMIC_DATABASE_ODBC=1)
endif ()

if (ATOMIC_IK)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS ik)
    target_compile_definitions(Atomic PUBLIC -DATOMIC_IK=1)
endif ()

target_include_directories (Atomic PUBLIC
    ${ATOMIC_SOURCE_DIR}/Source
    ${ATOMIC_SOURCE_DIR}/Source/ThirdParty
)

include(AtomicDoc)

if ($ENV{ATOMIC_BUILD_DIST})
    target_compile_definitions(Atomic PUBLIC -DATOMIC_BUILD_DIST=1)
endif ()

if (MSVC)
    target_compile_options(Atomic PUBLIC $<$<CONFIG:Debug>:${ATOMIC_MSVC_RUNTIME}d> $<$<NOT:$<CONFIG:Debug>>:${ATOMIC_MSVC_RUNTIME}>)
endif ()

target_link_libraries(Atomic 
    PUBLIC ${PUBLIC_THIRD_PARTY_DEPS}
    PRIVATE ${PRIVATE_THIRD_PARTY_DEPS})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_target_properties(Atomic PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)