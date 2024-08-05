cmake_minimum_required(VERSION 3.22.1)

set (CMAKE_CXX_STANDARD 20)
project(REngine)

set(ENGINE_SOURCE_DIR ${REngine_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${ENGINE_SOURCE_DIR}/Build/CMake/Modules)

include(EngineGlobals)
include(EngineGit)
include(EngineUtils)
include(EngineCommon)
include(EngineOptions)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set (ENGINE_RELEASE_OFF OFF)
    set (ENGINE_RELEASE_ON ON)
else ()
    set (ENGINE_RELEASE_OFF ON)
    set (ENGINE_RELEASE_ON OFF)
endif ()

add_definitions(-DENGINE_ROOT_SOURCE_DIR="${ENGINE_SOURCE_DIR}" -DENGINE_ROOT_BUILD_DIR="${CMAKE_BINARY_DIR}")

if (ENGINE_DEV_BUILD)
    add_definitions("-DENGINE_DEV_BUILD=1")
endif ()

if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "VULKAN_SDK environment variable is required to build REngine.")
endif()

set(VULKAN_SDK $ENV{VULKAN_SDK})

include(EnginePlatform)

find_program(CLDOC cldoc)
if (CLDOC)
    add_custom_target(docs DEPENDS EngineDocs)
endif ()

add_subdirectory(Source)

if (ENGINE_SAMPLES)
    initialize_submodule(${ENGINE_SOURCE_DIR}/Submodules/EngineExamples ENGINE_CPLUSPLUS_EXAMPLES)
    if (ENGINE_CPLUSPLUS_EXAMPLES)
        add_subdirectory(Submodules/EngineExamples/FeatureExamples/CPlusPlus)
    endif ()
endif ()