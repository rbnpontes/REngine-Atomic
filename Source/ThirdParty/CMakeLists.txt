#
# Portions Copyright (c) 2017 the Atomic project.
# Portions Copyright (c) 2024 the REngine project.
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

# When porting build scripts of third party libs please be aware:
# * removed all instances of `install_header_files()` macro calls. Do not bring them back.
# * added extra `set (INCLUDE_DIRS ...)`. Do not remove them.
# * added extra `target_link_libraries(...)`. Do not remove them.
# * added extra `target_compile_definitions(...)`. Do not remove them.
# * added extra `target_include_directories(...)`. Do not remove them.
# * If file contains a license with copyright attributed to REngine project - this file is written and maintained by
#   REngine team and should not be synced with upstream.

add_subdirectory(Box2D)
add_subdirectory(Duktape)
add_subdirectory(FreeType)
add_subdirectory(JO)
add_subdirectory(LZ4)
add_subdirectory(PugiXml)
add_subdirectory(STB)
add_subdirectory(zlib)

if (NOT ENGINE_BUILD_2D)
    add_subdirectory(Bullet)
    add_subdirectory(StanHull)
endif()
    
add_subdirectory(SDL)
    
add_subdirectory(TurboBadger)
add_subdirectory(Recast)
add_subdirectory(Detour)
add_subdirectory(DetourCrowd)
add_subdirectory(DetourTileCache)
add_subdirectory(eastl)

if (NOT WEB)
    add_subdirectory(Civetweb)
    add_subdirectory(kNet)
endif()

if (NOT IOS AND NOT ANDROID AND NOT WEB)
    add_subdirectory(Assimp)
    if (NOT LINUX)
        add_subdirectory(LibCpuId)
    endif()
    add_subdirectory(Poco)
    if (ENGINE_EDITOR)
        add_subdirectory(nativefiledialog)
    endif ()
    add_subdirectory(libsquish)
endif ()

if (ENGINE_DATABASE_SQLITE)
    add_subdirectory(SQLite)
endif ()
if (ENGINE_DATABASE_ODBC)
    add_subdirectory(nanodbc)
endif ()

add_subdirectory(rapidjson)

if (ENGINE_IK)
    add_subdirectory(ik)
endif ()

add_subdirectory(imgui)

if (ENGINE_PROFILING)
    add_subdirectory(tracy)
endif ()

add_subdirectory(spirv-headers)
add_subdirectory(spirv-tools)
add_subdirectory(glslang)
add_subdirectory(libshaderc_util)
add_subdirectory(libshaderc)

add_subdirectory(DiligentCore)

if (ENGINE_WEBVIEW)
    add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)
endif()