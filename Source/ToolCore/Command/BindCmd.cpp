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

#include <EngineCore/Core/Context.h>
#include <EngineCore/Core/StringUtils.h>
#include <EngineCore/IO/Log.h>
#include <EngineCore/IO/File.h>
#include <EngineCore/IO/FileSystem.h>

#include "../ToolSystem.h"
#include "../ToolEnvironment.h"

#include "../JSBind/JSBind.h"

#include "BindCmd.h"

namespace ToolCore
{

BindCmd::BindCmd(Context* context) : Command(context)
{

}

BindCmd::~BindCmd()
{

}

bool BindCmd::ParseInternal(const Vector<String>& arguments, unsigned startIndex, String& errorMsg)
{
    String argument = arguments[startIndex].ToLower();
    sourceRootFolder_ = startIndex + 1 < arguments.Size() ? arguments[startIndex + 1] : String::EMPTY;
    packageFolder_ = startIndex + 2 < arguments.Size() ? arguments[startIndex + 2] : String::EMPTY;

    if (argument != "bind")
    {
        errorMsg = "Unable to parse bind command";
        return false;
    }

    if (!sourceRootFolder_.Length())
    {
        errorMsg = "Unable to parse bind command";
        return false;
    }


    if (!packageFolder_.Length())
    {
        errorMsg = "Unable to parse bind command";
        return false;
    }

    sourceRootFolder_ = AddTrailingSlash(sourceRootFolder_);
    packageFolder_ = AddTrailingSlash(packageFolder_);

    return true;
}

void BindCmd::Run()
{
    SharedPtr<JSBind> jsbind(new JSBind(context_));

    context_->RegisterSubsystem(jsbind);

    ATOMIC_LOGINFOF("Loading Package");
    jsbind->LoadPackage(sourceRootFolder_, packageFolder_);

    jsbind->GenerateJavaScriptBindings();

    jsbind->GenerateCSharpBindings();

    Finished();

}

}
