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
#include <EngineCore/IO/File.h>
#include <EngineCore/IO/FileSystem.h>
#include <EngineCore/Resource/JSONFile.h>

#include "JSBind.h"
#include "JSBEvent.h"
#include "JSBClass.h"
#include "JSBModule.h"
#include "JSBPackage.h"
#include "JSBPackageWriter.h"

namespace ToolCore
{

Vector<SharedPtr<JSBPackage> > JSBPackage::allPackages_;

JSBPackage::JSBPackage(Context* context) : Object(context)
{
    // by default we bind for both JavaScript and C#
    bindingTypes_.Push(JAVASCRIPT);
    bindingTypes_.Push(CSHARP);
}

JSBPackage::~JSBPackage()
{

}

void JSBPackage::PreprocessModules()
{
    for (unsigned i = 0; i < modules_.Size(); i++)
    {
        modules_[i]->PreprocessHeaders();
    }
}

void JSBPackage::ProcessModules()
{
    for (unsigned i = 0; i < modules_.Size(); i++)
    {
        modules_[i]->VisitHeaders();
    }

    for (unsigned i = 0; i < modules_.Size(); i++)
    {
        modules_[i]->PreprocessClasses();
    }

    for (unsigned i = 0; i < modules_.Size(); i++)
    {
        modules_[i]->ProcessClasses();
    }

    for (unsigned i = 0; i < modules_.Size(); i++)
    {
        modules_[i]->PostProcessClasses();
    }

    for (unsigned i = 0; i < modules_.Size(); i++)
    {
        JSBEvent::ScanModuleEvents(modules_[i]);
    }

}

void JSBPackage::GenerateSource(JSBPackageWriter& packageWriter)
{
    packageWriter.GenerateSource();
    packageWriter.PostProcess();
}

JSBClass* JSBPackage::GetClass(const String& name, bool includeInterfaces)
{
    for (unsigned i = 0; i < modules_.Size(); i++)
    {
        JSBClass* cls = modules_[i]->GetClass(name, includeInterfaces);
        if (cls)
            return cls;
    }

    return 0;
}

PODVector<JSBClass*> JSBPackage::GetAllClasses(bool includeInterfaces)
{
    PODVector<JSBClass*> retVector;

    for (unsigned i = 0; i < allClasses_.Size(); i++)
    {
        if (!includeInterfaces && allClasses_[i]->IsInterface())
            continue;

        retVector.Push(allClasses_[i]);
    }

    return retVector;

}

JSBClass* JSBPackage::GetClassAllPackages(const String& name, bool includeInterfaces)
{
    for (unsigned i = 0; i < allPackages_.Size(); i++)
    {
        JSBClass* cls = allPackages_[i]->GetClass(name, includeInterfaces);
        if (cls)
            return cls;
    }

    return 0;

}

JSBEvent* JSBPackage::GetEvent(const String& eventID, const String& eventName)
{
    for (unsigned i = 0; i < modules_.Size(); i++)
    {
        JSBEvent* event = modules_[i]->GetEvent(eventID, eventName);

        if (event)
            return event;
    }

    return 0;
}

JSBEvent* JSBPackage::GetEventAllPackages(const String& eventID, const String& eventName)
{
    for (unsigned i = 0; i < allPackages_.Size(); i++)
    {
        JSBEvent* event = allPackages_[i]->GetEvent(eventID, eventName);
        if (event)
            return event;
    }

    return 0;
}

JSBEnum* JSBPackage::GetEnum(const String& name)
{
    for (unsigned i = 0; i < modules_.Size(); i++)
    {
        JSBEnum* cls = modules_[i]->GetEnum(name);
        if (cls)
            return cls;
    }

    return 0;
}

JSBEnum* JSBPackage::GetEnumAllPackages(const String& name)
{
    for (unsigned i = 0; i < allPackages_.Size(); i++)
    {
        JSBEnum* cls = allPackages_[i]->GetEnum(name);
        if (cls)
            return cls;
    }

    return 0;

}

bool JSBPackage::ContainsConstant(const String& constantName)
{
    for (unsigned i = 0; i < modules_.Size(); i++)
        if (modules_[i]->ContainsConstant(constantName))
            return true;

    return false;
}

bool JSBPackage::ContainsConstantAllPackages(const String& constantName)
{
    for (unsigned i = 0; i < allPackages_.Size(); i++)
    {
        if (allPackages_[i]->ContainsConstant(constantName))
            return true;
    }

    return false;

}

bool JSBPackage::Load(const String& packageFolder)
{
    ATOMIC_LOGINFOF("Loading Package: %s", packageFolder.CString());

    JSBind* jsbind = GetSubsystem<JSBind>();

    SharedPtr<File> jsonFile(new File(context_, packageFolder + "Package.json"));

    if (!jsonFile->IsOpen())
    {
        ATOMIC_LOGERRORF("Unable to open package json: %s", (packageFolder + "Package.json").CString());
        return false;
    }

    SharedPtr<JSONFile> packageJSON(new JSONFile(context_));

    if (!packageJSON->BeginLoad(*jsonFile))
    {
        ATOMIC_LOGERRORF("Unable to parse package json: %s", (packageFolder + "Package.json").CString());
        return false;
    }

    JSONValue& root = packageJSON->GetRoot();

    // first load dependencies
    JSONValue deps = root.Get("dependencies");

    if (deps.IsArray())
    {
        for (unsigned i = 0; i < deps.GetArray().Size(); i++)
        {
            String dpackageFolder = AddTrailingSlash(deps.GetArray()[i].GetString());

            SharedPtr<JSBPackage> depPackage (new JSBPackage(context_));

            if (!depPackage->Load(jsbind->GetSourceRootFolder() + dpackageFolder))
            {
                ATOMIC_LOGERRORF("Unable to load package dependency: %s", dpackageFolder.CString());
                return false;
            }

        }

    }

    JSONArray jplatforms = root["platforms"].GetArray();

    for (unsigned i = 0; i < jplatforms.Size(); i++)
    {
        platforms_.Push(jplatforms[i].GetString());
    }

    JSONValue jmodulesExclude = root.Get("moduleExclude");

    if (jmodulesExclude.IsObject())
    {
        Vector<String> platforms = jmodulesExclude.GetObject().Keys();

        for (unsigned i = 0; i < platforms.Size(); i++)
        {
            const String& platform = platforms[i];

            if (!moduleExcludes_.Contains(platform))
            {
                moduleExcludes_[platform] = Vector<String>();
            }

            JSONValue mods = jmodulesExclude.Get(platform);

            if (mods.IsArray())
            {

                for (unsigned j = 0; j < mods.GetArray().Size(); j++)
                {
                    moduleExcludes_[platform].Push(mods.GetArray()[j].GetString());
                }

            }

        }

    }

    JSONValue dnmodules = root.Get("dotnetModules");
    Vector<String> dotNetModules;
    if (dnmodules.IsArray())
    {
        for (unsigned i = 0; i < dnmodules.GetArray().Size(); i++)
        {
            String moduleName = dnmodules.GetArray()[i].GetString();
            dotNetModules.Push(moduleName);
        }
    }


    name_ = root.Get("name").GetString();
    namespace_ = root.Get("namespace").GetString();

    JSONValue modules = root.Get("modules");

    for (unsigned i = 0; i < modules.GetArray().Size(); i++)
    {
        String moduleName = modules.GetArray()[i].GetString();

        SharedPtr<JSBModule> module(new JSBModule(context_, this));

        if (!module->Load(packageFolder + moduleName + ".json"))
        {
            ATOMIC_LOGERRORF("Unable to load module json: %s", (packageFolder + moduleName + ".json").CString());
            return false;
        }

        if (dotNetModules.Contains(moduleName))
        {
            module->SetDotNetModule(true);
        }

        modules_.Push(module);

    }

    // bindings to generate
    JSONValue bindings = root.Get("bindings");

    if (bindings.IsArray())
    {
        bindingTypes_.Clear();

        for (unsigned i = 0; i < bindings.GetArray().Size(); i++)
        {
            String binding = bindings.GetArray()[i].GetString();

            if (binding.ToUpper() == "CSHARP")
            {
                bindingTypes_.Push(CSHARP);
            }
            else if (binding.ToUpper() == "JAVASCRIPT")
            {
                bindingTypes_.Push(JAVASCRIPT);
            }
        }
    }

    allPackages_.Push(SharedPtr<JSBPackage>(this));

    PreprocessModules();
    ProcessModules();

    return true;
}

String JSBPackage::GetPlatformDefineGuard() const
{
    if (!platforms_.Size())
        return String::EMPTY;

    StringVector defines;

    for (unsigned i = 0; i < platforms_.Size(); i++)
    {
        const String& platform = platforms_[i];

        if (platform.ToLower() == "windows")
            defines.Push("defined(ENGINE_PLATFORM_WINDOWS)");
        else if (platform.ToLower() == "macosx")
            defines.Push("defined(ENGINE_PLATFORM_MACOS)");
        else if (platform.ToLower() == "linux")
            defines.Push("defined(ENGINE_PLATFORM_LINUX)");
        else if (platform.ToLower() == "android")
            defines.Push("defined(ATOMIC_PLATFORM_ANDROID)");
        else if (platform.ToLower() == "ios")
            defines.Push("defined(ATOMIC_PLATFORM_IOS)");
        else if (platform.ToLower() == "web")
            defines.Push("defined(ATOMIC_PLATFORM_WEB)");
        else
        {
            ATOMIC_LOGERRORF("Unknown package platform: %s", platform.CString());
        }
    }

    if (!defines.Size())
        return String::EMPTY;

    String defineString = "#if " + String::Joined(defines, " || ");

    return defineString;

}

}
