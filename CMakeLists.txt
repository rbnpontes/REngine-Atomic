set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")
set (CMAKE_CXX_STANDARD 20)

project(Atomic)

cmake_minimum_required(VERSION 3.22.1)

set(ATOMIC_SOURCE_DIR ${Atomic_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${ATOMIC_SOURCE_DIR}/Build/CMake/Modules)

include(AtomicGit)
include(AtomicUtils)
include(AtomicCommon)
include(EngineOptions)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set (ATOMIC_RELEASE_OFF OFF)
    set (ATOMIC_RELEASE_ON ON)
else ()
    set (ATOMIC_RELEASE_OFF ON)
    set (ATOMIC_RELEASE_ON OFF)
endif ()

add_definitions(-DATOMIC_ROOT_SOURCE_DIR="${ATOMIC_SOURCE_DIR}" -DATOMIC_ROOT_BUILD_DIR="${CMAKE_BINARY_DIR}")

if (RENGINE_DEV_BUILD)
    add_definitions("-DATOMIC_DEV_BUILD=1 -DRENGINE_DEV_BUILD=1")
endif ()

if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "VULKAN_SDK environment variable is required to build REngine.")
endif()

set(VULKAN_SDK $ENV{VULKAN_SDK})

include(AtomicPlatform)

find_program(CLDOC cldoc)
if (CLDOC)
    add_custom_target(docs DEPENDS AtomicEngineDocs)
endif ()

add_subdirectory(Source)

if (ATOMIC_DEV_BUILD)
    if (NOT DEFINED ATOMIC_CPLUSPLUS_EXAMPLES)
        set(ATOMIC_CPLUSPLUS_EXAMPLES 1)
    endif ()
    if (ATOMIC_CPLUSPLUS_EXAMPLES)
        initialize_submodule(${ATOMIC_SOURCE_DIR}/Submodules/EngineExamples ATOMIC_CPLUSPLUS_EXAMPLES)
        if (ATOMIC_CPLUSPLUS_EXAMPLES)
            add_subdirectory(Submodules/EngineExamples/FeatureExamples/CPlusPlus)
        endif ()
    endif ()
endif ()