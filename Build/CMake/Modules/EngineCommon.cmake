#
# Portions Copyright (c) 2008-2016 the Urho3D project.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

include(CMakeParseArguments)
include(TestBigEndian)

# Source environment
execute_process(COMMAND env OUTPUT_VARIABLE ENVIRONMENT)
string(REGEX REPLACE "=[^\n]*\n?" ";" ENVIRONMENT "${ENVIRONMENT}")
set(IMPORT_ENGINE_VARIABLES_FROM_ENV LINUX APPLE IOS ANDROID WEB)
foreach(key ${ENVIRONMENT})
    list (FIND IMPORT_ENGINE_VARIABLES_FROM_ENV ${key} _index)
    if ("${key}" MATCHES "^(ENGINE_|CMAKE_).+" OR ${_index} GREATER -1)
        if (NOT DEFINED ${key})
            set (${key} $ENV{${key}} CACHE STRING "" FORCE)
        endif ()
    endif ()
endforeach()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DENGINE_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DENGINE_DEBUG")

if (CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(ENGINE_PROJECT_ARCH "x86_64")
    set(ENGINE_PROJECT_ARCH_SHORT "x64")
    set(ENGINE_PROJECT_ARCH_BITS "64")
    set(ENGINE_64BIT 1)
else ()
    message(FATAL_ERROR "32 bit architecture isn't supported")
endif ()

# Macro for defining source files with optional arguments as follows:
#  GLOB_CPP_PATTERNS <list> - Use the provided globbing patterns for CPP_FILES instead of the default *.cpp
#  GLOB_H_PATTERNS <list> - Use the provided globbing patterns for H_FILES instead of the default *.h
#  EXCLUDE_PATTERNS <list> - Use the provided patterns for excluding matched source files
#  EXTRA_CPP_FILES <list> - Include the provided list of files into CPP_FILES result
#  EXTRA_H_FILES <list> - Include the provided list of files into H_FILES result
#  PCH <list> - Enable precompiled header support on the defined source files using the specified header file, the list is "<path/to/header> [C++|C]"
#  PARENT_SCOPE - Glob source files in current directory but set the result in parent-scope's variable ${DIR}_CPP_FILES and ${DIR}_H_FILES instead
#  RECURSE - Option to glob recursively
#  GROUP - Option to group source files based on its relative path to the corresponding parent directory (only works when PARENT_SCOPE option is not in use)
macro(define_source_files)
    # Source files are defined by globbing source files in current source directory and also by including the extra source files if provided
    cmake_parse_arguments(ARG "PARENT_SCOPE;RECURSE;GROUP" "" "PCH;EXTRA_CPP_FILES;EXTRA_H_FILES;GLOB_CPP_PATTERNS;GLOB_H_PATTERNS;EXCLUDE_PATTERNS" ${ARGN})
    if (NOT ARG_GLOB_CPP_PATTERNS)
        set(ARG_GLOB_CPP_PATTERNS *.cpp)    # Default glob pattern
    endif ()
    if (NOT ARG_GLOB_H_PATTERNS)
        set(ARG_GLOB_H_PATTERNS *.h)
    endif ()
    if (ARG_RECURSE)
        set(ARG_RECURSE _RECURSE)
    else ()
        unset(ARG_RECURSE)
    endif ()
    file(GLOB${ARG_RECURSE} CPP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ARG_GLOB_CPP_PATTERNS})
    file(GLOB${ARG_RECURSE} H_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ARG_GLOB_H_PATTERNS})
    if (ARG_EXCLUDE_PATTERNS)
        set(CPP_FILES_WITH_SENTINEL ";${CPP_FILES};")  # Stringify the lists
        set(H_FILES_WITH_SENTINEL ";${H_FILES};")
        foreach (PATTERN ${ARG_EXCLUDE_PATTERNS})
            foreach (LOOP RANGE 1)
                string(REGEX REPLACE ";${PATTERN};" ";;" CPP_FILES_WITH_SENTINEL "${CPP_FILES_WITH_SENTINEL}")
                string(REGEX REPLACE ";${PATTERN};" ";;" H_FILES_WITH_SENTINEL "${H_FILES_WITH_SENTINEL}")
            endforeach ()
        endforeach ()
        set(CPP_FILES ${CPP_FILES_WITH_SENTINEL})      # Convert strings back to lists, extra sentinels are harmless
        set(H_FILES ${H_FILES_WITH_SENTINEL})
    endif ()
    list(APPEND CPP_FILES ${ARG_EXTRA_CPP_FILES})
    list(APPEND H_FILES ${ARG_EXTRA_H_FILES})
    set(SOURCE_FILES ${CPP_FILES} ${H_FILES})

    # Optionally enable PCH
    if (ARG_PCH)
        enable_pch(${ARG_PCH})
    endif ()

    # Optionally accumulate source files at parent scope
    if (ARG_PARENT_SCOPE)
        get_filename_component(NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
        set(${NAME}_CPP_FILES ${CPP_FILES} PARENT_SCOPE)
        set(${NAME}_H_FILES ${H_FILES} PARENT_SCOPE)
        # Optionally put source files into further sub-group (only works when PARENT_SCOPE option is not in use)
    elseif (ARG_GROUP)
        foreach (CPP_FILE ${CPP_FILES})
            get_filename_component(PATH ${CPP_FILE} PATH)
            if (PATH)
                string(REPLACE / \\ PATH ${PATH})
                source_group("Source Files\\${PATH}" FILES ${CPP_FILE})
            endif ()
        endforeach ()
        foreach (H_FILE ${H_FILES})
            get_filename_component(PATH ${H_FILE} PATH)
            if (PATH)
                string(REPLACE / \\ PATH ${PATH})
                source_group("Header Files\\${PATH}" FILES ${H_FILE})
            endif ()
        endforeach ()
    endif ()
endmacro()


# Macro for setting up dependency lib for compilation and linking of a target
macro(setup_target)
    # Include directories
    target_include_directories(${TARGET_NAME} SYSTEM BEFORE PUBLIC ${INCLUDE_DIRS})
    # Link libraries
    target_link_libraries(${TARGET_NAME} ${ABSOLUTE_PATH_LIBS} ${LIBS})
    # Enable PCH if requested
    if (${TARGET_NAME}_HEADER_PATHNAME)
        enable_pch(${${TARGET_NAME}_HEADER_PATHNAME})
    endif ()
    # Set additional linker dependencies (only work for Makefile-based generator according to CMake documentation)
    if (LINK_DEPENDS)
        string(REPLACE ";" "\;" LINK_DEPENDS "${LINK_DEPENDS}")        # Stringify for string replacement
        list(APPEND TARGET_PROPERTIES LINK_DEPENDS "${LINK_DEPENDS}")  # Stringify with semicolons already escaped
        unset(LINK_DEPENDS)
    endif ()
    if (TARGET_PROPERTIES)
        set_target_properties(${TARGET_NAME} PROPERTIES ${TARGET_PROPERTIES})
        unset(TARGET_PROPERTIES)
    endif ()
endmacro()

# Macro for checking the SOURCE_FILES variable is properly initialized
macro(check_source_files)
    if (NOT SOURCE_FILES)
        message(FATAL_ERROR "Could not configure and generate the project file because no source files have been defined yet. "
            "You can define the source files explicitly by setting the SOURCE_FILES variable in your CMakeLists.txt; or "
            "by calling the define_source_files() macro which would by default glob all the C++ source files found in the same scope of "
            "CMakeLists.txt where the macro is being called and the macro would set the SOURCE_FILES variable automatically. "
            "If your source files are not located in the same directory as the CMakeLists.txt or your source files are "
            "more than just C++ language then you probably have to pass in extra arguments when calling the macro in order to make it works. "
            "See the define_source_files() macro definition in the CMake/Modules/Urho3D-CMake-common.cmake for more detail.")
    endif ()
endmacro()

# Macro for setting up a library target
# Macro arguments:
#  NODEPS - setup library target without defining Urho3D dependency libraries (applicable for downstream projects)
#  STATIC/SHARED/MODULE/EXCLUDE_FROM_ALL - see CMake help on add_library() command
# CMake variables:
#  SOURCE_FILES - list of source files
#  INCLUDE_DIRS - list of directories for include search path
#  LIBS - list of dependent libraries that are built internally in the project
#  ABSOLUTE_PATH_LIBS - list of dependent libraries that are external to the project
#  LINK_DEPENDS - list of additional files on which a target binary depends for linking (Makefile-based generator only)
#  TARGET_PROPERTIES - list of target properties
macro(setup_library)
    cmake_parse_arguments(ARG NODEPS "" "" ${ARGN})
    check_source_files()
    add_library(${TARGET_NAME} ${ARG_UNPARSED_ARGUMENTS} ${SOURCE_FILES})
    get_target_property(LIB_TYPE ${TARGET_NAME} TYPE)

    setup_target()
endmacro()

# Macro for setting up an executable target
# Macro arguments:
#  PRIVATE - setup executable target without installing it
#  TOOL - setup a tool executable target
#  NODEPS - setup executable target without defining Urho3D dependency libraries
#  WIN32/MACOSX_BUNDLE/EXCLUDE_FROM_ALL - see CMake help on add_executable() command
# CMake variables:
#  SOURCE_FILES - list of source files
#  INCLUDE_DIRS - list of directories for include search path
#  LIBS - list of dependent libraries that are built internally in the project
#  ABSOLUTE_PATH_LIBS - list of dependent libraries that are external to the project
#  LINK_DEPENDS - list of additional files on which a target binary depends for linking (Makefile-based generator only)
#  TARGET_PROPERTIES - list of target properties
macro(setup_executable)
    cmake_parse_arguments(ARG "PRIVATE;TOOL;NODEPS" "" "" ${ARGN})
    check_source_files()
    add_executable(${TARGET_NAME} ${ARG_UNPARSED_ARGUMENTS} ${SOURCE_FILES})
    setup_target()
    if (ARG_TOOL)
        if (DEFINED ENGINE_TOOL_DIR)
            set (TOOL_DIR ${ENGINE_TOOL_DIR})
        else ()
            set (TOOL_DIR ${ENGINE_SOURCE_DIR}/Artifacts/Build/${TARGET_NAME})
        endif ()
        set_property(TARGET ${TARGET_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${TOOL_DIR})
    endif ()
endmacro()

# Macro for replacing substrings in every variable specified in the list.
# Macro arguments:
#  substring - a value that is to be replaced.
#  replacement - a new value that will replace `substring`.
#  variable_list - a list of variables. If list is specified manually enclose it in quotes and separate items with semicolon.
macro(replace_in_list substring replacement variable_list)
    foreach (single_variable ${variable_list})
        string(REPLACE "${substring}" "${replacement}" ${single_variable} "${${single_variable}}")
    endforeach ()
endmacro()

function(yarn_get_command)
    if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
        set(YARN_CMD "${ENGINE_SOURCE_DIR}/Build/Windows/yarn-forward.bat" PARENT_SCOPE)
    elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin")
        set(YARN_CMD "sh" "${ENGINE_SOURCE_DIR}/Build/Mac/yarn-forward.sh" PARENT_SCOPE)
    else()
        set(YARN_CMD "yarn" "--cwd" "${ENGINE_SOURCE_DIR}/Build" PARENT_SCOPE)
    endif()
endfunction()

macro(execute_yarn)
    if(NOT DEFINED YARN_WORKING_DIR)
        set(YARN_WORKING_DIR "${ENGINE_SOURCE_DIR}/Build")
    endif()

    yarn_get_command()
    if(DEFINED YARN_ARGS)
        set(YARN_ARGS ${YARN_CMD} ${YARN_ARGS})
    else()
        set(YARN_ARGS ${YARN_CMD})
    endif()

    execute_process(
        COMMAND ${YARN_ARGS}
        WORKING_DIRECTORY "${YARN_WORKING_DIR}"
        RESULTS_VARIABLE YARN_RESULT
    )
    if(NOT YARN_RESULT STREQUAL "0")
        message(FATAL_ERROR "Failed to run yarn command. Args: ${YARN_ARGS}")
    endif()
endmacro()

function(create_package resource_dir output_path)
    message(STATUS "Creating package for: ${output_path}")
    TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
    if (IS_BIG_ENDIAN)
        set(pak_endianess "1")
    else()
        set(pak_endianess "0")
    endif()

    set (YARN_ARGS "pkg" "${resource_dir}" "${output_path}" "${pak_endianess}")
    execute_yarn()
endfunction()
# Macro for setting msvc runtime flags globally.
# Macro arguments:
#  runtime_flag - release build runtime flag, /MT or /MD. Debug flag will be deduced automatically by appending 'd'.
macro(msvc_set_runtime runtime_flag)
    set(COMPILER_DEBUG_VARS "CMAKE_C_FLAGS_DEBUG;CMAKE_CXX_FLAGS_DEBUG")
    set(COMPILER_RELEASE_VARS "CMAKE_C_FLAGS_RELEASE;CMAKE_C_FLAGS_RELWITHDEBINFO;CMAKE_C_FLAGS_MINSIZEREL;CMAKE_CXX_FLAGS_RELEASE;CMAKE_CXX_FLAGS_RELWITHDEBINFO;CMAKE_CXX_FLAGS_MINSIZEREL")
    set(COMPILER_COMMON_VARS "CMAKE_C_FLAGS;CMAKE_CXX_FLAGS")
    set(COMPILER_RUNTIME_FLAGS "/MDd;/MD;/MTd;/MT")
    # Clear old runtime flags.
    foreach(flag ${COMPILER_RUNTIME_FLAGS})
        replace_in_list(${flag} "" "${COMPILER_DEBUG_VARS};${COMPILER_RELEASE_VARS};${COMPILER_COMMON_VARS}")
    endforeach()
    # Add release runtime flags.
    foreach(var ${COMPILER_RELEASE_VARS})
        set(${var} "${${var}} ${runtime_flag}")
    endforeach()
    # Add debug runtime flags.
    foreach(var ${COMPILER_DEBUG_VARS})
        set(${var} "${${var}} ${runtime_flag}d")
    endforeach()
endmacro()

macro(vs_add_to_grp target_name grp_name)
    set_target_properties("${target_name}" PROPERTIES FOLDER "${grp_name}")
endmacro()