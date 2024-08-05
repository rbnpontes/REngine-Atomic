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

set(OS_LINUX 1)
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
# Linux configuration.
#

# Platform-specific compiler/linker flags.
set(CEF_LIBTYPE SHARED)
list(APPEND CEF_COMPILER_FLAGS
  -fno-strict-aliasing            # Avoid assumptions regarding non-aliasing of objects of different types
  -fPIC                           # Generate position-independent code for shared libraries
  -fstack-protector               # Protect some vulnerable functions from stack-smashing (security feature)
  -funwind-tables                 # Support stack unwinding for backtrace()
  -fvisibility=hidden             # Give hidden visibility to declarations that are not explicitly marked as visible
  --param=ssp-buffer-size=4       # Set the minimum buffer size protected by SSP (security feature, related to stack-protector)
  -pipe                           # Use pipes rather than temporary files for communication between build stages
  -pthread                        # Use the pthread library
  -Wall                           # Enable all warnings
  -Wno-missing-field-initializers # Don't warn about missing field initializers
  -Wno-unused-parameter           # Don't warn about unused parameters
  -Wno-error=comment              # Don't warn about code in comments
  -Wno-comment                    # Don't warn about code in comments
  -Wno-deprecated-declarations    # Don't warn about using deprecated methods
  )
list(APPEND CEF_C_COMPILER_FLAGS
  -std=c99                        # Use the C99 language standard
  )
list(APPEND CEF_CXX_COMPILER_FLAGS
  -fexceptions                    # Enable exceptions
  -fno-threadsafe-statics         # Don't generate thread-safe statics
  -fvisibility-inlines-hidden     # Give hidden visibility to inlined class member functions
  -std=c++2a                      # Use the C++20 language standard
  -Wsign-compare                  # Warn about mixed signed/unsigned type comparisons
  )
list(APPEND CEF_COMPILER_FLAGS_DEBUG
  -O0                             # Disable optimizations
  -g                              # Generate debug information
  )
list(APPEND CEF_COMPILER_FLAGS_RELEASE
  -O2                             # Optimize for maximum speed
  -fdata-sections                 # Enable linker optimizations to improve locality of reference for data sections
  -ffunction-sections             # Enable linker optimizations to improve locality of reference for function sections
  -fno-ident                      # Ignore the #ident directive
  -U_FORTIFY_SOURCE               # Undefine _FORTIFY_SOURCE in case it was previously defined
  -D_FORTIFY_SOURCE=2             # Add memory and string function protection (security feature, related to stack-protector)
  )
list(APPEND CEF_LINKER_FLAGS
  -fPIC                           # Generate position-independent code for shared libraries
  -pthread                        # Use the pthread library
  -Wl,--disable-new-dtags         # Don't generate new-style dynamic tags in ELF
  -Wl,--fatal-warnings            # Treat warnings as errors
  -Wl,-rpath,.                    # Set rpath so that libraries can be placed next to the executable
  -Wl,-z,noexecstack              # Mark the stack as non-executable (security feature)
  -Wl,-z,now                      # Resolve symbols on program start instead of on first use (security feature)
  -Wl,-z,relro                    # Mark relocation sections as read-only (security feature)
  )
list(APPEND CEF_LINKER_FLAGS_RELEASE
  -Wl,-O1                         # Enable linker optimizations
  -Wl,--as-needed                 # Only link libraries that export symbols used by the binary
  -Wl,--gc-sections               # Remove unused code resulting from -fdata-sections and -function-sections
  )
list(APPEND CEF_COMPILER_DEFINES
  _FILE_OFFSET_BITS=64            # Allow the Large File Support (LFS) interface to replace the old interface
  )
list(APPEND CEF_COMPILER_DEFINES_RELEASE
  NDEBUG                          # Not a debug build
  )

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG(-Wno-undefined-var-template COMPILER_SUPPORTS_NO_UNDEFINED_VAR_TEMPLATE)
if(COMPILER_SUPPORTS_NO_UNDEFINED_VAR_TEMPLATE)
  list(APPEND CEF_CXX_COMPILER_FLAGS
    -Wno-undefined-var-template   # Don't warn about potentially uninstantiated static members
    )
endif()

CHECK_C_COMPILER_FLAG(-Wno-unused-local-typedefs COMPILER_SUPPORTS_NO_UNUSED_LOCAL_TYPEDEFS)
if(COMPILER_SUPPORTS_NO_UNUSED_LOCAL_TYPEDEFS)
  list(APPEND CEF_C_COMPILER_FLAGS
    -Wno-unused-local-typedefs  # Don't warn about unused local typedefs
    )
endif()

CHECK_CXX_COMPILER_FLAG(-Wno-literal-suffix COMPILER_SUPPORTS_NO_LITERAL_SUFFIX)
if(COMPILER_SUPPORTS_NO_LITERAL_SUFFIX)
  list(APPEND CEF_CXX_COMPILER_FLAGS
    -Wno-literal-suffix         # Don't warn about invalid suffixes on literals
    )
endif()

CHECK_CXX_COMPILER_FLAG(-Wno-narrowing COMPILER_SUPPORTS_NO_NARROWING)
if(COMPILER_SUPPORTS_NO_NARROWING)
  list(APPEND CEF_CXX_COMPILER_FLAGS
    -Wno-narrowing              # Don't warn about type narrowing
    )
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  list(APPEND CEF_CXX_COMPILER_FLAGS
    -Wno-attributes             # The cfi-icall attribute is not supported by the GNU C++ compiler
    )
endif()

if(PROJECT_ARCH STREQUAL "x86_64")
  # 64-bit architecture.
  list(APPEND CEF_COMPILER_FLAGS
    -m64
    -march=x86-64
    )
  list(APPEND CEF_LINKER_FLAGS
    -m64
    )
elseif(PROJECT_ARCH STREQUAL "x86")
  # 32-bit architecture.
  list(APPEND CEF_COMPILER_FLAGS
    -msse2
    -mfpmath=sse
    -mmmx
    -m32
    )
  list(APPEND CEF_LINKER_FLAGS
    -m32
    )
endif()

# Standard libraries.
set(CEF_STANDARD_LIBS
  X11
  )

# CEF directory paths.
set(CEF_RESOURCE_DIR        "${ENGINE_SOURCE_DIR}/Artifacts/CEF/Resources")
set(CEF_BINARY_DIR          "${ENGINE_SOURCE_DIR}/Artifacts/CEF/Linux")
set(CEF_BINARY_DIR_DEBUG    "${ENGINE_SOURCE_DIR}/Artifacts/CEF/Linux")
set(CEF_BINARY_DIR_RELEASE  "${ENGINE_SOURCE_DIR}/Artifacts/CEF/Linux")

# CEF library paths.
set(CEF_LIB_DEBUG   "${CEF_BINARY_DIR_DEBUG}/libcef.so")
set(CEF_LIB_RELEASE "${CEF_BINARY_DIR_RELEASE}/libcef.so")

# List of CEF binary files.
set(CEF_BINARY_FILES
  libcef.so
  libEGL.so
  libGLESv2.so
  libvk_swiftshader.so
  libvulkan.so.1
  snapshot_blob.bin
  v8_context_snapshot.bin
  vk_swiftshader_icd.json
  )

# List of CEF resource files.
set(CEF_RESOURCE_FILES
  chrome_100_percent.pak
  chrome_200_percent.pak
  resources.pak
  icudtl.dat
  locales
  )

if(USE_SANDBOX)
  list(APPEND CEF_COMPILER_DEFINES
    CEF_USE_SANDBOX   # Used by apps to test if the sandbox is enabled
    )
endif()