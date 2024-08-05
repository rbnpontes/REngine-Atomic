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

#pragma once

#include <EngineCore/IPC/IPCServer.h>

#include "AssetImporter.h"

using namespace Atomic;

namespace ToolCore
{
    class NETAssemblyImporter;

    class NETAssemblyImporterResultHandler : public IPCResultHandler
    {
        ATOMIC_OBJECT(NETAssemblyImporterResultHandler, IPCResultHandler)

    public:
        /// Construct.
        NETAssemblyImporterResultHandler(Context* context, NETAssemblyImporter* importer);
        /// Destruct.
        virtual ~NETAssemblyImporterResultHandler();

        virtual void HandleResult(unsigned cmdID, const VariantMap& cmdResult);

    private:

        WeakPtr<NETAssemblyImporter> importer_;

    };

    class NETAssemblyImporter : public AssetImporter
    {
        friend class NETAssemblyImporterResultHandler;

        ATOMIC_OBJECT(NETAssemblyImporter, AssetImporter)

    public:
        /// Construct.
        NETAssemblyImporter(Context* context, Asset* asset);
        virtual ~NETAssemblyImporter();

        virtual void SetDefaults();

        Resource* GetResource(const String& typeName = String::EMPTY);

    protected:

        bool Import();

        virtual bool LoadSettingsInternal(JSONValue& jsonRoot);
        virtual bool SaveSettingsInternal(JSONValue& jsonRoot);

        void GetAssetCacheMap(HashMap<String, String>& assetMap);

    private:

        virtual void HandleResult(unsigned cmdID, const VariantMap& cmdResult);

        bool SaveAssemblyCacheFile();

        JSONValue assemblyJSON_;

        SharedPtr<NETAssemblyImporterResultHandler> resultHandler_;

    };

}
