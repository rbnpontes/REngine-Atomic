# Copyright (c) 2016 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

# Must be loaded via FindCEF.cmake.
if(NOT DEFINED _CEF_ROOT_EXPLICIT)
  message(FATAL_ERROR "Use find_package(CEF) to load this file.")
endif()


#
# Shared configuration.
#
set(OS_MAC 1)
set(OS_MACOSX 1)  # For backwards compatibility.
set(OS_POSIX 1)

# Determine the project architecture.
if(NOT DEFINED PROJECT_ARCH)
  if(("${CMAKE_HOST_SYSTEM_PROCESSOR}" STREQUAL "arm64") OR
     ("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "ARM64"))
    set(PROJECT_ARCH "arm64")
  elseif(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PROJECT_ARCH "x86_64")
  else()
    set(PROJECT_ARCH "x86")
  endif()
endif()

if(${CMAKE_GENERATOR} STREQUAL "Ninja")
  set(GEN_NINJA 1)
elseif(${CMAKE_GENERATOR} STREQUAL "Unix Makefiles")
  set(GEN_MAKEFILES 1)
endif()

# Determine the build type.
if(NOT CMAKE_BUILD_TYPE AND (GEN_NINJA OR GEN_MAKEFILES))
  # CMAKE_BUILD_TYPE should be specified when using Ninja or Unix Makefiles.
  set(CMAKE_BUILD_TYPE Release)
  message(WARNING "No CMAKE_BUILD_TYPE value selected, using ${CMAKE_BUILD_TYPE}")
endif()


# Path to the include directory.
set(CEF_INCLUDE_PATH "${_CEF_ROOT}")

# Path to the libcef_dll_wrapper target.
set(CEF_LIBCEF_DLL_WRAPPER_PATH "${_CEF_ROOT}/libcef_dll")


# Shared compiler/linker flags.
list(APPEND CEF_COMPILER_DEFINES
  # Allow C++ programs to use stdint.h macros specified in the C99 standard that aren't 
  # in the C++ standard (e.g. UINT8_MAX, INT64_MIN, etc)
  __STDC_CONSTANT_MACROS __STDC_FORMAT_MACROS
  )


# Configure use of the sandbox.
set(USE_SANDBOX FALSE)


#
# Mac OS X configuration.
#

# Platform-specific compiler/linker flags.
# See also Xcode target properties in cef_macros.cmake.
set(CEF_LIBTYPE SHARED)
list(APPEND CEF_COMPILER_FLAGS
  -fno-strict-aliasing            # Avoid assumptions regarding non-aliasing of objects of different types
  -fstack-protector               # Protect some vulnerable functions from stack-smashing (security feature)
  -funwind-tables                 # Support stack unwinding for backtrace()
  -fvisibility=hidden             # Give hidden visibility to declarations that are not explicitly marked as visible
  -Wall                           # Enable all warnings
  -Werror                         # Treat warnings as errors
  -Wextra                         # Enable additional warnings
  -Wendif-labels                  # Warn whenever an #else or an #endif is followed by text
  -Wnewline-eof                   # Warn about no newline at end of file
  -Wno-missing-field-initializers # Don't warn about missing field initializers
  -Wno-unused-parameter           # Don't warn about unused parameters
  )
list(APPEND CEF_C_COMPILER_FLAGS
  -std=c99                        # Use the C99 language standard
  )
list(APPEND CEF_CXX_COMPILER_FLAGS
  -fno-exceptions                 # Disable exceptions
  -fno-rtti                       # Disable real-time type information
  -fno-threadsafe-statics         # Don't generate thread-safe statics
  -fobjc-call-cxx-cdtors          # Call the constructor/destructor of C++ instance variables in ObjC objects
  -fvisibility-inlines-hidden     # Give hidden visibility to inlined class member functions
  -std=c++17                      # Use the C++17 language standard
  -Wno-narrowing                  # Don't warn about type narrowing
  -Wsign-compare                  # Warn about mixed signed/unsigned type comparisons
  )
list(APPEND CEF_COMPILER_FLAGS_DEBUG
  -O0                             # Disable optimizations
  -g                              # Generate debug information
  )
list(APPEND CEF_COMPILER_FLAGS_RELEASE
  -O3                             # Optimize for maximum speed plus a few extras
  )
list(APPEND CEF_LINKER_FLAGS
  -Wl,-search_paths_first         # Search for static or shared library versions in the same pass
  -Wl,-ObjC                       # Support creation of ObjC static libraries
  -Wl,-pie                        # Generate position-independent code suitable for executables only
  )
list(APPEND CEF_LINKER_FLAGS_RELEASE
  -Wl,-dead_strip                 # Strip dead code
  )

include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG(-Wno-undefined-var-template COMPILER_SUPPORTS_NO_UNDEFINED_VAR_TEMPLATE)
if(COMPILER_SUPPORTS_NO_UNDEFINED_VAR_TEMPLATE)
  list(APPEND CEF_CXX_COMPILER_FLAGS
    -Wno-undefined-var-template   # Don't warn about potentially uninstantiated static members
    )
endif()

# Standard libraries.
set(CEF_STANDARD_LIBS
  -lpthread
  "-framework AppKit"
  "-framework Cocoa"
  "-framework IOSurface"
  )

# Find the newest available base SDK.
execute_process(COMMAND xcode-select --print-path OUTPUT_VARIABLE XCODE_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
foreach(OS_VERSION 14.2 14.0 10.15)
  set(SDK "${XCODE_PATH}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${OS_VERSION}.sdk")
  if(NOT "${CMAKE_OSX_SYSROOT}" AND EXISTS "${SDK}" AND IS_DIRECTORY "${SDK}")
    set(CMAKE_OSX_SYSROOT ${SDK})
  endif()
endforeach()

# Target SDK.
set(CEF_TARGET_SDK               "10.15")
list(APPEND CEF_COMPILER_FLAGS
  -mmacosx-version-min=${CEF_TARGET_SDK}
)
set(CMAKE_OSX_DEPLOYMENT_TARGET  ${CEF_TARGET_SDK})

# Target architecture.
if(PROJECT_ARCH STREQUAL "x86_64")
  set(CMAKE_OSX_ARCHITECTURES "x86_64")
elseif(PROJECT_ARCH STREQUAL "arm64")
  set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
  set(CMAKE_OSX_ARCHITECTURES "i386")
endif()

# Prevent Xcode 11 from doing automatic codesigning.
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")

# CEF directory paths.
set(CEF_BINARY_DIR          "${ATOMIC_SOURCE_DIR}/Artifacts/CEF/MacOSX-${PROJECT_ARCH}")
set(CEF_BINARY_DIR_DEBUG    "${ATOMIC_SOURCE_DIR}/Artifacts/CEF/MacOSX-${PROJECT_ARCH}")
set(CEF_BINARY_DIR_RELEASE  "${ATOMIC_SOURCE_DIR}/Artifacts/CEF/MacOSX-${PROJECT_ARCH}")

if(USE_SANDBOX)
  list(APPEND CEF_COMPILER_DEFINES
    CEF_USE_SANDBOX   # Used by apps to test if the sandbox is enabled
    )

  list(APPEND CEF_STANDARD_LIBS
    -lsandbox
    )

  # CEF sandbox library paths.
  set(CEF_SANDBOX_LIB_DEBUG "${CEF_BINARY_DIR_DEBUG}/cef_sandbox.a")
  set(CEF_SANDBOX_LIB_RELEASE "${CEF_BINARY_DIR_RELEASE}/cef_sandbox.a")
endif()

# CEF Helper app suffixes.
# Format is "<name suffix>:<target suffix>:<plist suffix>".
set(CEF_HELPER_APP_SUFFIXES
  "::"
  " (Alerts):_alerts:.alerts"
  " (GPU):_gpu:.gpu"
  " (Plugin):_plugin:.plugin"
  " (Renderer):_renderer:.renderer"
  )