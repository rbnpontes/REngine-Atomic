
# Copyright 2020 The Shaderc Authors. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# utility functions

function(shaderc_default_c_compile_options TARGET)
  if (NOT "${MSVC}")
    if (SHADERC_ENABLE_WERROR_COMPILE)
      target_compile_options(${TARGET} PRIVATE -Wall -Werror -fvisibility=hidden)
    else()
      target_compile_options(${TARGET} PRIVATE -Wall -fvisibility=hidden)
    endif()
    check_cxx_compiler_flag(-fPIC COMPILER_SUPPORTS_PIC)
    if (NOT "${MINGW}" AND COMPILER_SUPPORTS_PIC)
      target_compile_options(${TARGET} PRIVATE -fPIC)
    endif()
    if (ENABLE_CODE_COVERAGE)
      # The --coverage option is a synonym for -fprofile-arcs -ftest-coverage
      # when compiling.
      target_compile_options(${TARGET} PRIVATE -g -O0 --coverage)
      # The --coverage option is a synonym for -lgcov when linking for gcc.
      # For clang, it links in a different library, libclang_rt.profile, which
      # requires clang to be built with compiler-rt.
      target_link_libraries(${TARGET} PRIVATE --coverage)
    endif()
    if (NOT SHADERC_ENABLE_SHARED_CRT)
      if (WIN32)
        # For MinGW cross compile, statically link to the libgcc runtime.
        # But it still depends on MSVCRT.dll.
        set_target_properties(${TARGET} PROPERTIES
          LINK_FLAGS "-static -static-libgcc")
      endif(WIN32)
    endif(NOT SHADERC_ENABLE_SHARED_CRT)
  else()
    # disable warning C4800: 'int' : forcing value to bool 'true' or 'false'
    # (performance warning)
    target_compile_options(${TARGET} PRIVATE /wd4800)
  endif()
endfunction(shaderc_default_c_compile_options)

function(shaderc_default_compile_options TARGET)
  shaderc_default_c_compile_options(${TARGET})
  if (NOT "${MSVC}")
    if (NOT SHADERC_ENABLE_SHARED_CRT)
      if (WIN32)
        # For MinGW cross compile, statically link to the C++ runtime.
        # But it still depends on MSVCRT.dll.
        set_target_properties(${TARGET} PROPERTIES
          LINK_FLAGS "-static -static-libgcc -static-libstdc++")
      endif(WIN32)
    endif(NOT SHADERC_ENABLE_SHARED_CRT)
  endif()
endfunction(shaderc_default_compile_options)

# Build an asciidoc file; additional arguments past the base filename specify
# additional dependencies for the file.
function(shaderc_add_asciidoc TARGET FILE)
  if (ASCIIDOCTOR_EXE)
    set(DEST ${CMAKE_CURRENT_BINARY_DIR}/${FILE}.html)
    add_custom_command(
      COMMAND ${ASCIIDOCTOR_EXE} -a toc -o ${DEST}
        ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}.asciidoc
      DEPENDS ${FILE}.asciidoc ${ARGN}
      OUTPUT ${DEST})
    # Create the target, but the default build target does not depend on it.
    # Some Asciidoctor installations are mysteriously broken, and it's hard
    # to detect those cases.  Generating HTML is not critical by default.
    add_custom_target(${TARGET} DEPENDS ${DEST})
  endif(ASCIIDOCTOR_EXE)
endfunction()

# Finds all transitive static library dependencies of a given target
# including possibly the target itself.
# This will skip libraries that were statically linked that were not
# built by CMake, for example -lpthread.
macro(shaderc_get_transitive_libs target out_list)
  if (TARGET ${target})
    get_target_property(libtype ${target} TYPE)
    # If this target is a static library, get anything it depends on.
    if ("${libtype}" STREQUAL "STATIC_LIBRARY")
      # Get the original library if this is an alias library. This is
      # to avoid putting both the original library and the alias library
      # in the list (given we are deduplicating according to target names).
      # Otherwise, we may pack the same library twice, resulting in
      # duplicated symbols.
      get_target_property(aliased_target ${target} ALIASED_TARGET)
      if (aliased_target)
        list(INSERT ${out_list} 0 "${aliased_target}")
      else()
        list(INSERT ${out_list} 0 "${target}")
      endif()

      get_target_property(libs ${target} LINK_LIBRARIES)
      if (libs)
        foreach(lib ${libs})
          shaderc_get_transitive_libs(${lib} ${out_list})
        endforeach()
      endif()
    endif()
  endif()
  # If we know the location (i.e. if it was made with CMake) then we
  # can add it to our list.
  LIST(REMOVE_DUPLICATES ${out_list})
endmacro()

# Combines the static library "target" with all of its transitive static
# library dependencies into a single static library "new_target".
function(shaderc_combine_static_lib new_target target)
  set(all_libs "")
  shaderc_get_transitive_libs(${target} all_libs)
  add_library(${new_target} STATIC)
  foreach(lib IN LISTS all_libs)
    target_sources(${new_target} PRIVATE $<TARGET_OBJECTS:${lib}>)
  endforeach()
endfunction()