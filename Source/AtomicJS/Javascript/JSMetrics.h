//
// Copyright (c) 2014-2015, THUNDERBEAST GAMES LLC All rights reserved
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

#include <EngineCore/Core/Context.h>
#include <EngineCore/Core/Object.h>
#include <EngineCore/Container/List.h>

namespace Atomic
{

class JSVM;

class JSMetrics : public Object
{
    ATOMIC_OBJECT(JSMetrics, Object);

    struct ObjectMetric
    {
        String classname;
        int count;
    };

    struct NodeMetric
    {
        String name;
        int count;
    };


public:

    /// Construct.
    JSMetrics(Context* context, JSVM* vm);
    /// Destruct.
    virtual ~JSMetrics();

    void Capture();

    void Dump();
    void DumpNodes();
    void DumpJSComponents();

private:

    WeakPtr<JSVM> vm_;

    // Object
    HashMap<StringHash, ObjectMetric> objectMetrics_;

    // Nodes
    HashMap<StringHash, NodeMetric> nodeMetrics_;

};

}
