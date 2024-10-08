//
// Copyright (c) 2014-2017, THUNDERBEAST GAMES LLC All rights reserved
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


/// JavaScript/TypeScript Debugger Subsystem
class JSDebugger : public Object
{

    ATOMIC_OBJECT(JSDebugger, Object)

public:

    /// Construct.
    JSDebugger(Context* context);

    /// Destruct.
    virtual ~JSDebugger();

    /// Reconnect to debugger socket (blocks main thread currently)
    void Reconnect() const;

    /// Shut down the debugger
    void Shutdown() const;

    /// Get the JSDebugger subsystem from external code
    static JSDebugger* GetInstance() { return instance_; }

    /// Set the Auto Reconnect value
    void SetAutoReconnect(bool value) { autoReconnect_ = value; }

    /// Get the Auto Reconnect value
    bool GetAutoReconnect() { return autoReconnect_; }


private:

    static JSDebugger* instance_;
    bool autoReconnect_;

};

}
