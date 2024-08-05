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

#include <EngineCore/Core/Object.h>

namespace Atomic
{

    /// Reference to CSAssembly made
    ATOMIC_EVENT(E_CSCOMPONENTASSEMBLYREFERENCE, CSComponentAssemblyReference)
    {
        ATOMIC_PARAM(P_ASSEMBLYPATH, AssemblyPath); // String
    }

    ATOMIC_EVENT(E_CSCOMPONENTLOAD, CSComponentLoad)
    {
        ATOMIC_PARAM(P_CLASSNAME, ClassName); // String
        ATOMIC_PARAM(P_NATIVEINSTANCE, NativeInstance); // CSComponent as void*
        ATOMIC_PARAM(P_FIELDVALUES, FieldValues);  // VariantMap as void*
    }

    ATOMIC_EVENT(E_CSCOMPONENTASSEMBLYCHANGED, CSComponentAssemblyChanged)
    {
        ATOMIC_PARAM(P_RESOURCE, Resource); // Resource
        ATOMIC_PARAM(P_ASSEMBLYPATH, AssemblyPath); // String
    }

    ATOMIC_EVENT(E_CSCOMPONENTCLASSCHANGED, CSComponentClassChanged)
    {
        ATOMIC_PARAM(P_CSCOMPONENT, Component); // CSComponent*
        ATOMIC_PARAM(P_CLASSNAME, Classname); // String
    }

}
