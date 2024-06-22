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

# Determine the platform.
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  set(OS_MAC 1)
  set(OS_MACOSX 1)  # For backwards compatibility.
  set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(OS_LINUX 1)
  set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  set(OS_WINDOWS 1)
endif()

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
option(USE_SANDBOX "Enable or disable use of the sandbox." ON)

#
# Windows configuration.
#

if(OS_WINDOWS)
  if (GEN_NINJA)
    # When using the Ninja generator clear the CMake defaults to avoid excessive
    # console warnings (see issue #2120).
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
  endif()

  if(USE_SANDBOX)
    # Check if the current MSVC version is compatible with the cef_sandbox.lib
    # static library. We require VS2015 or newer.
    if(MSVC_VERSION LESS 1900)
      message(WARNING "CEF sandbox is not compatible with the current MSVC version (${MSVC_VERSION})")
      set(USE_SANDBOX OFF)
    endif()
  endif()

  # Consumers who run into LNK4099 warnings can pass /Z7 instead (see issue #385).
  set(CEF_DEBUG_INFO_FLAG "/Zi" CACHE STRING "Optional flag specifying specific /Z flag to use")

  # Consumers using different runtime types may want to pass different flags
  set(CEF_RUNTIME_LIBRARY_FLAG "/MT" CACHE STRING "Optional flag specifying which runtime to use")
  if (CEF_RUNTIME_LIBRARY_FLAG)
    list(APPEND CEF_COMPILER_FLAGS_DEBUG ${CEF_RUNTIME_LIBRARY_FLAG}d)
    list(APPEND CEF_COMPILER_FLAGS_RELEASE ${CEF_RUNTIME_LIBRARY_FLAG})
  endif()

  # Platform-specific compiler/linker flags.
  set(CEF_LIBTYPE STATIC)
  list(APPEND CEF_COMPILER_FLAGS
    /MP           # Multiprocess compilation
    /Gy           # Enable function-level linking
    /GR-          # Disable run-time type information
    /W4           # Warning level 4
    /WX           # Treat warnings as errors
    /wd4100       # Ignore "unreferenced formal parameter" warning
    /wd4127       # Ignore "conditional expression is constant" warning
    /wd4244       # Ignore "conversion possible loss of data" warning
    /wd4324       # Ignore "structure was padded due to alignment specifier" warning
    /wd4481       # Ignore "nonstandard extension used: override" warning
    /wd4512       # Ignore "assignment operator could not be generated" warning
    /wd4701       # Ignore "potentially uninitialized local variable" warning
    /wd4702       # Ignore "unreachable code" warning
    /wd4996       # Ignore "function or variable may be unsafe" warning
    ${CEF_DEBUG_INFO_FLAG}
    )
  list(APPEND CEF_COMPILER_FLAGS_DEBUG
    /RTC1         # Disable optimizations
    /Od           # Enable basic run-time checks
    )
  list(APPEND CEF_COMPILER_FLAGS_RELEASE
    /O2           # Optimize for maximum speed
    /Ob2          # Inline any suitable function
    /GF           # Enable string pooling
    )
  list(APPEND CEF_CXX_COMPILER_FLAGS
    /std:c++20    # Use the C++17 language standard
    )
  list(APPEND CEF_LINKER_FLAGS_DEBUG
    /DEBUG        # Generate debug information
    )
  list(APPEND CEF_EXE_LINKER_FLAGS
    /MANIFEST:NO        # No default manifest (see ADD_WINDOWS_MANIFEST macro usage)
    /LARGEADDRESSAWARE  # Allow 32-bit processes to access 3GB of RAM

    # Delayload most libraries as the dlls are simply not required at startup (or
    # at all, depending on the process type). Some dlls open handles when they are
    # loaded, and we may not want them to be loaded in renderers or other sandboxed
    # processes. Conversely, some dlls must be loaded before sandbox lockdown. In
    # unsandboxed processes they will load when first needed. The linker will
    # automatically ignore anything which is not linked to the binary at all (it is
    # harmless to have an unmatched /delayload). This list should be kept in sync
    # with Chromium's "delayloads" target from the //build/config/win/BUILD.gn file.
    /DELAYLOAD:api-ms-win-core-winrt-error-l1-1-0.dll
    /DELAYLOAD:api-ms-win-core-winrt-l1-1-0.dll
    /DELAYLOAD:api-ms-win-core-winrt-string-l1-1-0.dll
    /DELAYLOAD:advapi32.dll
    /DELAYLOAD:comctl32.dll
    /DELAYLOAD:comdlg32.dll
    /DELAYLOAD:credui.dll
    /DELAYLOAD:cryptui.dll
    /DELAYLOAD:d3d11.dll
    /DELAYLOAD:d3d9.dll
    /DELAYLOAD:dwmapi.dll
    /DELAYLOAD:dxgi.dll
    /DELAYLOAD:dxva2.dll
    /DELAYLOAD:esent.dll
    /DELAYLOAD:gdi32.dll
    /DELAYLOAD:hid.dll
    /DELAYLOAD:imagehlp.dll
    /DELAYLOAD:imm32.dll
    /DELAYLOAD:msi.dll
    /DELAYLOAD:netapi32.dll
    /DELAYLOAD:ncrypt.dll
    /DELAYLOAD:ole32.dll
    /DELAYLOAD:oleacc.dll
    /DELAYLOAD:propsys.dll
    /DELAYLOAD:psapi.dll
    /DELAYLOAD:rpcrt4.dll
    /DELAYLOAD:rstrtmgr.dll
    /DELAYLOAD:setupapi.dll
    /DELAYLOAD:shell32.dll
    /DELAYLOAD:shlwapi.dll
    /DELAYLOAD:uiautomationcore.dll
    /DELAYLOAD:urlmon.dll
    /DELAYLOAD:user32.dll
    /DELAYLOAD:usp10.dll
    /DELAYLOAD:uxtheme.dll
    /DELAYLOAD:wer.dll
    /DELAYLOAD:wevtapi.dll
    /DELAYLOAD:wininet.dll
    /DELAYLOAD:winusb.dll
    /DELAYLOAD:wsock32.dll
    /DELAYLOAD:wtsapi32.dll
    )
  list(APPEND CEF_COMPILER_DEFINES
    WIN32 _WIN32 _WINDOWS             # Windows platform
    UNICODE _UNICODE                  # Unicode build
    # Targeting Windows 10. We can't say `=_WIN32_WINNT_WIN10` here because
    # some files do `#if WINVER < 0x0600` without including windows.h before,
    # and then _WIN32_WINNT_WIN10 isn't yet known to be 0x0A00.
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00
    NTDDI_VERSION=NTDDI_WIN10_FE
    NOMINMAX                          # Use the standard's templated min/max
    WIN32_LEAN_AND_MEAN               # Exclude less common API declarations
    _HAS_EXCEPTIONS=0                 # Disable exceptions
    )
  list(APPEND CEF_COMPILER_DEFINES_RELEASE
    NDEBUG _NDEBUG                    # Not a debug build
    )

  if(PROJECT_ARCH STREQUAL "x86")
    # Set the initial stack size to 0.5MiB, instead of the 1.5MiB minimum
    # needed by CEF's main thread. This saves significant memory on threads
    # (like those in the Windows thread pool, and others) whose stack size we
    # can only control through this setting. The main thread (in 32-bit builds
    # only) uses fibers to switch to a 4MiB stack at runtime via
    # CefRunWinMainWithPreferredStackSize().
    list(APPEND CEF_EXE_LINKER_FLAGS
      /STACK:0x80000
      )
  else()
    # Increase the initial stack size to 8MiB from the default 1MiB.
    list(APPEND CEF_EXE_LINKER_FLAGS
      /STACK:0x800000
      )
  endif()

  # Standard libraries.
  set(CEF_STANDARD_LIBS
    comctl32.lib
    gdi32.lib
    rpcrt4.lib
    shlwapi.lib
    ws2_32.lib
    )

  # CEF directory paths.
  set(CEF_RESOURCE_DIR        "${_CEF_ROOT}/Resources")
  set(CEF_BINARY_DIR          "${_CEF_ROOT}/Release")
  set(CEF_BINARY_DIR_DEBUG    "${_CEF_ROOT}/Release")
  set(CEF_BINARY_DIR_RELEASE  "${_CEF_ROOT}/Release")

  # CEF library paths.
  set(CEF_LIB_DEBUG   "${CEF_BINARY_DIR_DEBUG}/libcef.lib")
  set(CEF_LIB_RELEASE "${CEF_BINARY_DIR_RELEASE}/libcef.lib")

  # List of CEF binary files.
  set(CEF_BINARY_FILES
    chrome_elf.dll
    d3dcompiler_47.dll
    libcef.dll
    libEGL.dll
    libGLESv2.dll
    snapshot_blob.bin
    v8_context_snapshot.bin
    vk_swiftshader.dll
    vk_swiftshader_icd.json
    vulkan-1.dll
    )

  if(PROJECT_ARCH STREQUAL "x86_64")
    list(APPEND CEF_BINARY_FILES
      dxil.dll
      dxcompiler.dll
      )
  endif()

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
      PSAPI_VERSION=1   # Required by cef_sandbox.lib
      CEF_USE_SANDBOX   # Used by apps to test if the sandbox is enabled
      )
    # Libraries required by cef_sandbox.lib.
    set(CEF_SANDBOX_STANDARD_LIBS
      Advapi32.lib
      dbghelp.lib
      Delayimp.lib
      ntdll.lib
      OleAut32.lib
      PowrProf.lib
      Propsys.lib
      psapi.lib
      SetupAPI.lib
      Shell32.lib
      Shcore.lib
      Userenv.lib
      version.lib
      wbemuuid.lib
      WindowsApp.lib
      winmm.lib
      )

    # CEF sandbox library paths.
    set(CEF_SANDBOX_LIB_DEBUG "${CEF_BINARY_DIR_DEBUG}/cef_sandbox.lib")
    set(CEF_SANDBOX_LIB_RELEASE "${CEF_BINARY_DIR_RELEASE}/cef_sandbox.lib")
  endif()

  # Configure use of ATL.
  option(USE_ATL "Enable or disable use of ATL." ON)
  if(USE_ATL)
    # Locate the atlmfc directory if it exists. It may be at any depth inside
    # the VC directory. The cl.exe path returned by CMAKE_CXX_COMPILER may also
    # be at different depths depending on the toolchain version
    # (e.g. "VC/bin/cl.exe", "VC/bin/amd64_x86/cl.exe",
    # "VC/Tools/MSVC/14.10.25017/bin/HostX86/x86/cl.exe", etc).
    set(HAS_ATLMFC 0)
    get_filename_component(VC_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    get_filename_component(VC_DIR_NAME ${VC_DIR} NAME)
    while(NOT ${VC_DIR_NAME} STREQUAL "VC")
      get_filename_component(VC_DIR ${VC_DIR} DIRECTORY)
      if(IS_DIRECTORY "${VC_DIR}/atlmfc")
        set(HAS_ATLMFC 1)
        break()
      endif()
      get_filename_component(VC_DIR_NAME ${VC_DIR} NAME)
    endwhile()

    # Determine if the Visual Studio install supports ATL.
    if(NOT HAS_ATLMFC)
      message(WARNING "ATL is not supported by your VC installation.")
      set(USE_ATL OFF)
    endif()
  endif()

  if(USE_ATL)
    list(APPEND CEF_COMPILER_DEFINES
      CEF_USE_ATL   # Used by apps to test if ATL support is enabled
      )
  endif()
endif()