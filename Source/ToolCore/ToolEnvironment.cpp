//
// Copyright (c) 2014-2016 THUNDERBEAST GAMES LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <EngineCore/IO/Log.h>
#include <EngineCore/IO/FileSystem.h>
#include <EngineCore/IO/File.h>

#include "ToolEnvironment.h"


namespace ToolCore
{

bool ToolEnvironment::bootstrapping_ = false;

ToolEnvironment::ToolEnvironment(Context* context) : Object(context),
    cli_(false),
    toolPrefs_(new ToolPrefs(context))
{

}

ToolEnvironment::~ToolEnvironment()
{

}

const String& ToolEnvironment::GetVsWhereBinary() {
#ifdef ENGINE_PLATFORM_WINDOWS
    return vs_where_binary_;
#else
    return "";
#endif
}

bool ToolEnvironment::InitFromDistribution()
{
    toolPrefs_->Load();

    FileSystem* fileSystem = GetSubsystem<FileSystem>();

#ifdef ENGINE_PLATFORM_WINDOWS
    editorBinary_ = fileSystem->GetProgramDir() + "EngineEditor.exe";
    String resourcesDir = fileSystem->GetProgramDir() + "Resources/";
    vs_where_binary_ = fileSystem->GetProgramDir() + "vswhere.exe";
    playerBinary_ = resourcesDir + ToString("ToolData/Deployment/Windows/x64/%s.exe", ENGINE_PLAYER_TARGET);
#elif ENGINE_PLATFORM_LINUX
    editorBinary_ = fileSystem->GetProgramDir() + "EngineEditor";
    String resourcesDir = fileSystem->GetProgramDir() + "Resources/";
    playerBinary_ = resourcesDir + ToString("ToolData/Deployment/Linux/%s", ENGINE_PLAYER_TARGET);
#else
    editorBinary_ = fileSystem->GetProgramDir() + "EngineEditor";
    String resourcesDir = GetPath(RemoveTrailingSlash(fileSystem->GetProgramDir())) + "Resources/";
    playerAppFolder_ = resourcesDir + ToString("ToolData/Deployment/MacOS/%s.app/", ENGINE_PLAYER_TARGET);
#endif

    resourceCoreDataDir_ = resourcesDir + "CoreData";
    resourcePlayerDataDir_ = resourcesDir + "PlayerData";

    toolDataDir_ =  resourcesDir + "ToolData/";

    // EngineNET

#ifdef ENGINE_DEBUG
    String config = "Debug";
#else
    String config = "Release";
#endif

    atomicNETRootDir_ = resourcesDir + ToString("ToolData/%s/", ENGINE_NET_NAME);
    atomicNETCoreAssemblyDir_ = atomicNETRootDir_ + config + "/";

#ifdef ENGINE_PLATFORM_MACOS
    monoExecutableDir_ = "/Library/Frameworks/Mono.framework/Versions/Current/Commands/";
    atomicNETNuGetBinary_ = monoExecutableDir_ + "nuget";
#endif

    return true;
}

bool ToolEnvironment::Initialize(bool cli)
{
    bool result = true;

    cli_ = cli;
    toolPrefs_->Load();

#ifdef ENGINE_DEV_BUILD

    SetRootSourceDir(ENGINE_ROOT_SOURCE_DIR);
    SetRootBuildDir(ENGINE_ROOT_BUILD_DIR, true);

#else

    if (!bootstrapping_)
    {
        result = InitFromDistribution();

    }
    else
    {
        SetRootSourceDir(ENGINE_ROOT_SOURCE_DIR);
        SetRootBuildDir(ENGINE_ROOT_BUILD_DIR, true);

    }
#endif

    return result;

}

void ToolEnvironment::SetRootSourceDir(const String& sourceDir)
{
    rootSourceDir_ = AddTrailingSlash(sourceDir);
    resourceCoreDataDir_ = rootSourceDir_ + "Resources/CoreData";
    resourcePlayerDataDir_ = rootSourceDir_ + "Resources/PlayerData";
    resourceEditorDataDir_ = rootSourceDir_ + "Resources/EditorData";
    toolDataDir_ = rootSourceDir_ + "Data/";

    // EngineNET

#ifdef ENGINE_DEBUG
    String config = "Debug";
#else
    String config = "Release";
#endif

    atomicNETRootDir_ = rootSourceDir_ + ToString("Artifacts/%s/", ENGINE_NET_NAME);
    atomicNETCoreAssemblyDir_ = rootSourceDir_ + ToString("Artifacts/%s/", ENGINE_NET_NAME) + config + "/";

#ifdef ENGINE_PLATFORM_WINDOWS
    vs_where_binary_ = ToString("%sArtifacts/vswhere.exe", rootSourceDir_.CString());
#endif

#if defined ENGINE_PLATFORM_WINDOWS || defined ENGINE_PLATFORM_LINUX
    atomicNETNuGetBinary_ = ToString("%sBuild/Managed/nuget/nuget.exe", rootSourceDir_.CString());
#endif

#ifdef ENGINE_PLATFORM_MACOS
    monoExecutableDir_ = "/Library/Frameworks/Mono.framework/Versions/Current/Commands/";
    atomicNETNuGetBinary_ = monoExecutableDir_ + "nuget";
#endif

}

void ToolEnvironment::SetRootBuildDir(const String& buildDir, bool setBinaryPaths)
{
    FileSystem* fileSystem = GetSubsystem<FileSystem>();
    rootBuildDir_ = AddTrailingSlash(buildDir);


    if (setBinaryPaths)
    {
        #ifdef ENGINE_PLATFORM_WINDOWS
            const char* build_type;
            #ifdef _DEBUG
                build_type = "Debug";
            #else
                build_type = "Release";
            #endif

            playerBinary_ = rootBuildDir_ + ToString("Source/%s/Application/%s/%s.exe", ENGINE_PLAYER_TARGET, build_type, ENGINE_PLAYER_TARGET);
            editorBinary_ = rootBuildDir_ + ToString("Source/%s/%s/%s.exe", ENGINE_EDITOR_NAME, build_type, ENGINE_EDITOR_NAME);

            // some build tools like ninja don't use Release/Debug folders
            if (!fileSystem->FileExists(playerBinary_))
                    playerBinary_ = rootBuildDir_ + ToString("Source/%s/Application/%s.exe", ENGINE_PLAYER_TARGET, ENGINE_PLAYER_TARGET);
            if (!fileSystem->FileExists(editorBinary_))
                    editorBinary_ = rootBuildDir_ + ToString("Source/%s/%s.exe", ENGINE_EDITOR_NAME, ENGINE_EDITOR_NAME);

            playerAppFolder_ = rootSourceDir_ + ToString("Data/Deployment/MacOS/%s.app", ENGINE_PLAYER_TARGET);
        #elif ENGINE_PLATFORM_MACOS
            #ifdef ENGINE_XCODE
                playerBinary_ = rootBuildDir_ + ToString(
                    "Source/%s/%s/%s.app/Contents/MacOS/%s", 
                    ENGINE_PLAYER_TARGET, CMAKE_INTDIR, 
                    ENGINE_PLAYER_TARGET, ENGINE_PLAYER_TARGET
                );
                editorBinary_ = rootBuildDir_ + ToString(
                    "Source/%s/%s/%s.app/Contents/MacOS/%s",
                    ENGINE_EDITOR_NAME,
                    CMAKE_INTDIR,
                    ENGINE_EDITOR_NAME,
                    ENGINE_EDITOR_NAME
                );
            #else
                playerBinary_ = rootBuildDir_ + ToString(
                    "Source/%s/Application/%s.app/Contents/MacOS/%s",
                    ENGINE_PLAYER_TARGET, ENGINE_PLAYER_TARGET, ENGINE_PLAYER_TARGET
                );
                playerAppFolder_ = rootBuildDir_ + ToString(
                    "Source/%s/Application/%s.app/",
                    ENGINE_PLAYER_TARGET, ENGINE_PLAYER_TARGET
                );
                editorBinary_ = rootBuildDir_ + ToString(
                    "Source/%s/%s.app/Contents/MacOS/%s",
                    ENGINE_EDITOR_NAME, ENGINE_EDITOR_NAME, ENGINE_EDITOR_NAME
                );
            #endif
        #else
            playerBinary_ = rootBuildDir_ + ToString(
                "Source/%s/Application/%s",
                ENGINE_PLAYER_TARGET, ENGINE_PLAYER_TARGET
            );
            editorBinary_ = rootBuildDir_ + ToString(
                "Source/%s/%s",
                ENGINE_EDITOR_NAME, ENGINE_EDITOR_NAME
            );
        #endif
    }

}

String ToolEnvironment::GetIOSDeployBinary()
{
    return GetToolDataDir() + "Deployment/IOS/ios-deploy/ios-deploy";
}

void ToolEnvironment::Dump()
{
    ATOMIC_LOGINFOF("Root Source Dir: %s", rootSourceDir_.CString());
    ATOMIC_LOGINFOF("Root Build Dir: %s", rootBuildDir_.CString());

    ATOMIC_LOGINFOF("Core Resource Dir: %s", resourceCoreDataDir_.CString());
    ATOMIC_LOGINFOF("Player Resource Dir: %s", resourcePlayerDataDir_.CString());
    ATOMIC_LOGINFOF("Editor Resource Dir: %s", resourceEditorDataDir_.CString());

    ATOMIC_LOGINFOF("Editor Binary: %s", editorBinary_.CString());
    ATOMIC_LOGINFOF("Player Binary: %s", playerBinary_.CString());
    ATOMIC_LOGINFOF("Tool Binary: %s", toolBinary_.CString());
#ifdef ENGINE_PLATFORM_WINDOWS
    ATOMIC_LOGINFOF("VsWhere Binary: %s", vs_where_binary_.CString());
#endif


    ATOMIC_LOGINFOF("Tool Data Dir: %s", toolDataDir_.CString());

    ATOMIC_LOGINFOF("Deployment Data Dir: %s", deploymentDataDir_.CString());

}

}
