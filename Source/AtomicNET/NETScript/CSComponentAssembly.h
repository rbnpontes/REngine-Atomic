//
// Copyright (c) 2014-2016, THUNDERBEAST GAMES LLC All rights reserved
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

#include <EngineCore/Resource/Resource.h>
#include <EngineCore/Resource/JSONValue.h>
#include <EngineCore/Container/ArrayPtr.h>
#include <EngineCore/Container/List.h>
#include <EngineCore/Script/ScriptComponentFile.h>

namespace Atomic
{

    class CSComponent;
    class NETVariantMap;

    /// NET Assembly resource.
    class CSComponentAssembly : public ScriptComponentFile
    {
        ATOMIC_OBJECT(CSComponentAssembly, ScriptComponentFile)

    public:

        /// Construct.
        CSComponentAssembly(Context* context);
        /// Destruct.
        virtual ~CSComponentAssembly();        

        CSComponent* CreateCSComponent(const String& classname);

        /// Load resource from stream. May be called from a worker thread. Return true if successful.
        virtual bool BeginLoad(Deserializer& source);
        /// Save resource
        virtual bool Save(Serializer& dest) const;

        /// Register object factory.
        static void RegisterObject(Context* context);

        const String& GetFullPath() const { return fullAssemblyPath_;  }

        /// Only valid in editor, as we don't inspect assembly at runtime
        const Vector<String>& GetClassNames() { return classNames_; }

        // Find assembly by class name or namespace qualified classname
        static CSComponentAssembly* ResolveClassAssembly(const String& fullClassName);

        // TODO: Proper method to preload class assemblies (which will also work on mobile)
        static bool PreloadClassAssemblies();

    private:

        bool ParseAssemblyJSON(const JSONValue& json);

        static void InitTypeMap();

        bool ParseComponentClassJSON(const JSONValue& json);

        String fullAssemblyPath_;

        HashMap<String, Vector<EnumInfo>> assemblyEnums_;
        static HashMap<StringHash, VariantType> typeMap_;

    };

}
