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

#include <EngineCore/IPC/IPC.h>
#include <EngineCore/IPC/IPCEvents.h>
#include <EngineCore/IPC/IPCBroker.h>

#include <EngineCore/Core/CoreEvents.h>

#include <ToolCore/ToolEnvironment.h>
#include <ToolCore/ToolSystem.h>

#include "../Command/NETCmd.h"

#include "AtomicNETService.h"

namespace ToolCore
{

    AtomicNETService::AtomicNETService(Context* context) :
        IPCServer(context)
    {

    }

    AtomicNETService::~AtomicNETService()
    {

    }

    bool AtomicNETService::GetServiceExecutable(String& execPath, Vector<String>& args) const
    {

        ToolEnvironment* tenv = GetSubsystem<ToolEnvironment>();

        execPath = String::EMPTY;
        args.Clear();

#ifdef ENGINE_DEBUG
        String config = "Debug";
#else
        String config = "Release";
#endif

        ea::vector<ea::string> files;
        const auto search_path = tenv->GetAtomicNETRootDir().ToStdString() + config.ToStdString();
        const auto target_net_service = ea::string(ENGINE_NET_SERVICE_NAME) + ".exe";
    	const auto file_system = GetSubsystem<FileSystem>();
        file_system->ScanDir(files, search_path, "*", SCAN_FILES | SCAN_DIRS, true);

        String netServiceFilename;
        for(const auto& file : files)
        {
            const auto target_it = file.find(target_net_service);
            if(target_it == ea::string::npos)
                continue;

            netServiceFilename = (search_path + "/" + file).c_str();
        }

#ifdef ENGINE_PLATFORM_WINDOWS        

        execPath = netServiceFilename;

#elif defined ENGINE_PLATFORM_MACOS

        execPath = tenv->GetMonoExecutableDir() + "mono64";
        args.Push(netServiceFilename);

#elif defined ENGINE_PLATFORM_LINUX

        execPath = "/usr/bin/mono";
        args.Push(netServiceFilename);
#endif

        if (!file_system->FileExists(execPath))
        {
            ATOMIC_LOGERRORF("AtomicNETService binary not found: %s", execPath.CString());
            return false;
        }

        return true;
    }

    bool AtomicNETService::Start()
    {
        String exec;
        Vector<String> args;

        if (!GetServiceExecutable(exec, args))
            return false;

        if (!IPCServer::StartInternal(exec, args))
        {
            return false;
        }

        return true;

    }

}
