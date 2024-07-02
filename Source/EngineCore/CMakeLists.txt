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
if(ENGINE_2D OR ENGINE_2D_ONLY)
    file (GLOB _2D_SOURCE 2D/*.cpp 2D/*.h)
endif()
file (GLOB SCENE_SOURCE Scene/*.cpp Scene/*.h)
file (GLOB UI_SOURCE UI/*.cpp UI/*.h)
file (GLOB SYSTEM_UI_SOURCE UI/SystemUI/*.cpp UI/SystemUI/*.h)
file (GLOB PHYSICS_SOURCE Physics/*.cpp Physics/*.h)
file (GLOB NAVIGATION_SOURCE Navigation/*.cpp Navigation/*.h)
file (GLOB ENVIRONMENT_SOURCE Environment/*.cpp Environment/*.h)
file (GLOB GRAPHICS_SOURCE Graphics/*.cpp Graphics/*.h Graphics/Text3D/*.cpp Graphics/Text3D/*.h)
file (GLOB RHI_SOURCE RHI/*.cpp RHI/*.h)

if (ENGINE_IK)
    file (GLOB IK_SOURCE IK/*.cpp IK/*.h)
endif ()

if (ENGINE_DATABASE_SQLITE)
    file (GLOB DATABASE_SOURCE Database/*.cpp Database/*.h Database/SQLite/*.cpp Database/SQLite/*.h)
elseif (ENGINE_DATABASE_ODBC)
    file (GLOB DATABASE_SOURCE Database/*.cpp Database/*.h Database/ODBC/*.cpp Database/ODBC/*.h)
endif ()

if (APPLE AND NOT IOS)
    set (PLATFORM_SOURCE UI/UIDragDropMac.mm)
    if(ENGINE_FILEWATCHER)
        list(APPEND PLATFORM_SOURCE IO/MacFileWatcher.mm)
    endif()
endif()

if(ANDROID)
    file(GLOB PLATFORM_SOURCE IO/Android/*.cpp IO/Android/*.h)
endif()

set (SOURCE_FILES ${CONTAINER_SOURCE} ${CORE_SOURCE} ${ENGINE_SOURCE} ${INPUT_SOURCE}
                  ${AUDIO_SOURCE} ${IO_SOURCE} ${MATH_SOURCE}
                  ${RESOURCE_SOURCE} ${PHYSICS_SOURCE} ${IPC_SOURCE}
                  ${GRAPHICS_SOURCE} ${GRAPHICS_IMPL_SOURCE}
                  ${_3D_SOURCE} ${_2D_SOURCE} ${ENVIRONMENT_SOURCE}
                  ${SCENE_SOURCE} ${UI_SOURCE} ${SYSTEM_UI_SOURCE}
                  ${SCRIPT_SOURCE} ${METRICS_SOURCE}
                  ${PLATFORM_SOURCE} ${DATABASE_SOURCE} ${IK_SOURCE} 
                  ${NAVIGATION_SOURCE} ${RHI_SOURCE})
if (NOT WEB)
  set (SOURCE_FILES ${SOURCE_FILES} ${NETWORK_SOURCE})
endif()

GroupSources("2D")
GroupSources("3D")
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

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Handle Git Revision
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/BuildInfo/EngineGitSHA.cpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/BuildInfo/EngineGitSHA.cpp" @ONLY)

set (SOURCE_FILES ${SOURCE_FILES} BuildInfo/EngineBuildInfo.cpp BuildInfo/EngineBuildInfo.h BuildInfo/EngineGitSHA.cpp BuildInfo/EngineGitSHA.h)

if (ENGINE_SHARED)
    set(ENGINE_LIBRARY_TYPE SHARED)
else ()
    set(ENGINE_LIBRARY_TYPE STATIC)
endif ()

add_library("${ENGINE_CORE_LIB_TARGET}" ${ENGINE_LIBRARY_TYPE} ${SOURCE_FILES})
set_target_properties("${ENGINE_CORE_LIB_TARGET}" PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

if (NOT ENGINE_SHARED)
    target_compile_definitions ("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_STATIC_DEFINE=1 -DENGINE_API=)
else ()
    target_compile_definitions ("${ENGINE_CORE_LIB_TARGET}"
        PRIVATE -DENGINE_EXPORTS=1 
    )
endif ()

if(ENGINE_SSE)
    target_compile_definitions("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_SSE=1)
endif ()

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories("${ENGINE_CORE_LIB_TARGET}" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)

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
set (PRIVATE_THIRD_PARTY_DEPS SDL2::SDL2-static shaderc)

target_compile_definitions ("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_CXX11=1 -DENGINE_64BIT=1)

if (ANDROID)
    list(APPEND PRIVATE_THIRD_PARTY_DEPS dl log android GLESv1_CM GLESv2)
    target_compile_definitions("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_PLATFORM_ANDROID=1)
elseif (APPLE)
    target_compile_definitions("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_PLATFORM_APPLE=1)
    list(APPEND PRIVATE_THIRD_PARTY_DEPS
            "-framework AudioToolbox"
            "-framework CoreAudio" 
            "-framework CoreVideo" 
            "-framework IOKit" 
            "-framework CoreServices"
            "-framework Security"
            "-framework SystemConfiguration")
    if (IOS)
        target_compile_definitions("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DIOS=1 -DENGINE_PLATFORM_IOS=1)
    else () # MacOS
        list(APPEND PRIVATE_THIRD_PARTY_DEPS 
            "-framework Cocoa"
            "-framework Carbon"
            "-framework ForceFeedback"
            "-framework OpenGL")
        target_compile_definitions("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_PLATFORM_MACOS=1)
    endif ()
elseif (LINUX)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS pthread dl)
    target_compile_definitions("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_PLATFORM_LINUX=1)
elseif (WIN32)
    target_compile_definitions ("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_PLATFORM_WINDOWS=1)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid Ws2_32)
    list(APPEND PRIVATE_THIRD_PARTY_DEPS Shcore)
elseif (WEB)
    target_compile_definitions("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_PLATFORM_WEB=1 -DNO_POPEN=1)
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

if (ENGINE_DESKTOP)
    target_compile_definitions ("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_PLATFORM_DESKTOP=1)
    list(APPEND PRIVATE_THIRD_PARTY_DEPS libsquish)
    if (NOT LINUX)
        list(APPEND PRIVATE_THIRD_PARTY_DEPS LibCpuId)
    endif()
    if (ENGINE_FILEWATCHER)
        target_compile_definitions("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_FILEWATCHER=1)
    endif ()
endif ()

if (NOT WEB)
    target_compile_definitions ("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_THREADING=1 -DENGINE_NETWORK=1 -DENGINE_WEB=1)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS Civetweb kNet)
endif()

if (ENGINE_PROFILING)
    target_compile_definitions("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_PROFILING=1)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS TracyClient)
endif ()

if (ENGINE_LOGGING)
    target_compile_definitions ("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_LOGGING=1)
endif ()

if (ENGINE_2D_ONLY)
    target_compile_definitions ("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_2D=1)
else ()
    target_compile_definitions ("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_3D=1)

    if (ENGINE_2D)
        target_compile_definitions ("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_2D=1)
    endif ()

    if (ENGINE_PHYSICS)
        target_compile_definitions ("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_PHYSICS=1)
        list(APPEND PUBLIC_THIRD_PARTY_DEPS Bullet StanHull)
    endif ()

    if (ENGINE_NAVIGATION)
        target_compile_definitions ("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_NAVIGATION=1)
        list(APPEND PUBLIC_THIRD_PARTY_DEPS Recast Detour DetourCrowd DetourTileCache)
    endif ()
endif ()

# TODO: enable shared library builds.
if (NOT APPLE OR IOS OR WEB)
    target_compile_definitions ("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_TBUI=1)
endif ()

if (ENGINE_DATABASE_SQLITE)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS sqlite)
    target_compile_definitions("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_DATABASE=1 -DENGINE_DATABASE_SQLITE=1)
elseif (ENGINE_DATABASE_ODBC)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS nanodbc)
    target_compile_definitions("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_DATABASE=1 -DENGINE_DATABASE_ODBC=1)
endif ()

if (ENGINE_IK)
    list(APPEND PUBLIC_THIRD_PARTY_DEPS ik)
    target_compile_definitions("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_IK=1)
endif ()

target_include_directories ("${ENGINE_CORE_LIB_TARGET}" PUBLIC
    ${ENGINE_SOURCE_DIR}/Source
    ${ENGINE_SOURCE_DIR}/Source/ThirdParty
)

include(EngineDoc)

if ($ENV{ENGINE_BUILD_DIST})
    target_compile_definitions("${ENGINE_CORE_LIB_TARGET}" PUBLIC -DENGINE_BUILD_DIST=1)
endif ()

if (MSVC)
    target_compile_options("${ENGINE_CORE_LIB_TARGET}" PUBLIC $<$<CONFIG:Debug>:${ENGINE_MSVC_RUNTIME}d> $<$<NOT:$<CONFIG:Debug>>:${ENGINE_MSVC_RUNTIME}>)
endif ()

target_link_libraries("${ENGINE_CORE_LIB_TARGET}"
    PUBLIC ${PUBLIC_THIRD_PARTY_DEPS}
    PRIVATE ${PRIVATE_THIRD_PARTY_DEPS})