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

#include <EngineCore/IO/File.h>
#include <EngineCore/IO/FileSystem.h>
#include <EngineCore/Core/StringUtils.h>

#include "../JSBind.h"
#include "../JSBFunction.h"
#include "../JSBModule.h"
#include "../JSBPackage.h"
#include "../JSBEnum.h"
#include "../JSBClass.h"

#include "CSTypeHelper.h"
#include "CSModuleWriter.h"
#include "CSPackageWriter.h"

namespace ToolCore
{

CSPackageWriter::CSPackageWriter(JSBPackage *package) : JSBPackageWriter(package)
{

}

void CSPackageWriter::GenNativeFunctionSignature(JSBFunction* function, String& sig)
{
    JSBClass* klass = function->GetClass();

    const Vector<JSBFunctionType*>& parameters = function->GetParameters();

    Vector<String> args;

    if (!function->IsConstructor())
    {
        args.Push(ToString("%s* self", klass->GetNativeName().CString()));
    }

    if (parameters.Size())
    {
        for (unsigned int i = 0; i < parameters.Size(); i++)
        {
            JSBFunctionType* ptype = parameters.At(i);

            // ignore "Context" parameters
            if (ptype->type_->asClassType())
            {
                JSBClassType* classType = ptype->type_->asClassType();
                JSBClass* klass = classType->class_;
                if (klass->GetName() == "Context")
                {
                    continue;
                }

                args.Push(ToString("%s* %s", klass->GetNativeName().CString(), ptype->name_.CString()));
            }
            else
            {
                args.Push(CSTypeHelper::GetNativeTypeString(ptype) + " " + ptype->name_);
            }

        }
    }

    if (function->GetReturnClass() && function->GetReturnClass()->IsNumberArray())
    {
        args.Push(ToString("%s* returnValue", function->GetReturnClass()->GetNativeName().CString()));
    }

    sig.Join(args, ", ");

}

void CSPackageWriter::GenerateNativeHeader()
{
    String source = "// This file was autogenerated by AtomicTool, changes will be lost\n\n";


    source += "#pragma once\n\nnamespace Atomic\n{\n}\n";

    /*

    if (package_->name_ != "Atomic")
    {
        source += "#include \"../../Atomic/Native/CSPackageAtomic.h\"\n";
        source += "using namespace Atomic;\n";
    }

    // enum includes

    PODVector<JSBHeader*> enumHeaders;
    for (unsigned i = 0; i < package_->modules_.Size(); i++)
    {
        Vector<SharedPtr<JSBEnum>> enums = package_->modules_[i]->GetEnums();
        for (unsigned j = 0; j < enums.Size(); j++)
        {
            JSBHeader* header = enums[j]->GetHeader();
            if (!enumHeaders.Contains(header))
                enumHeaders.Push(header);
        }
    }

    for (unsigned i = 0; i < enumHeaders.Size(); i++)
    {
        JSBHeader* header = enumHeaders[i];

        String headerPath = GetPath(header->GetFilePath());

        String headerfile = GetFileNameAndExtension(header->GetFilePath());

        JSBind* jsbind = header->GetSubsystem<JSBind>();

        headerPath.Replace(jsbind->GetSourceRootFolder() + "Source/", "");

        source.AppendWithFormat("#include <%s%s>\n", headerPath.CString(), headerfile.CString());

    }


    // forward declarations of package classes
    source += ToString("namespace %s\n{\n\n", package_->GetNamespace().CString());

    Indent();

    PODVector<JSBClass*>& allClasses = package_->GetAllClasses();
    for (unsigned i = 0; i < allClasses.Size(); i++)
    {
        JSBClass* cls = allClasses[i];
        source += IndentLine(ToString("class %s;\n", cls->GetNativeName().CString()));
    }

    Dedent();

    source += "\n}\n";
    */

    JSBind* jsbind = package_->GetSubsystem<JSBind>();

    String filepath = jsbind->GetDestNativeFolder() + "/CSPackage" + package_->name_ + ".h";

    File file(package_->GetContext());
    file.Open(filepath, FILE_WRITE);
    file.Write(source.CString(), source.Length());
    file.Close();

}

void CSPackageWriter::GenerateNativeSource()
{
    GenerateNativeHeader();

    String source = "// This file was autogenerated by AtomicTool, changes will be lost\n\n";

    String defineGuard = package_->GetPlatformDefineGuard();

    if (defineGuard.Length())
    {
        source += ToString("%s\n\n", defineGuard.CString());
    }

    const char* packageName = package_->name_.CString();

    String packageHeader = "CSPackage" + package_->name_ + ".h";

    source += ToString("#include \"%s\"\n", packageHeader.CString());

    if (package_->name_ != "Atomic")
        source += "using namespace Atomic;\n";

    source += ToString("namespace %s\n{\n", packageName);

    // end of namespace
    source += "\n}\n";

    if (defineGuard.Length())
    {
        source += "\n#endif\n";
    }

    JSBind* jsbind = package_->GetSubsystem<JSBind>();

    String filepath = jsbind->GetDestNativeFolder() + "/CSPackage" + package_->name_ + ".cpp";

    File file(package_->GetContext());
    file.Open(filepath, FILE_WRITE);
    file.Write(source.CString(), source.Length());
    file.Close();
}

void CSPackageWriter::GenerateManagedSource()
{

}

void CSPackageWriter::GenerateSource()
{

    GenerateNativeSource();
    GenerateManagedSource();

    for (unsigned i = 0; i < package_->modules_.Size(); i++)
    {
        CSModuleWriter writer(package_->modules_[i]);
        writer.GenerateSource();
    }

}

}
