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

#include "IPCTypes.h"
#include "IPC.h"
#include "IPCChannel.h"

namespace Atomic
{

class ATOMIC_API IPCWorker : public IPCChannel
{
    ATOMIC_OBJECT(IPCWorker, IPCChannel);

public:
    /// POSIX Constructor
    IPCWorker(Context* context, IPCHandle fd, unsigned id);

    // Windows Constructor, two named pipes are used
    IPCWorker(Context* context, IPCHandle clientRead, IPCHandle clientWrite, unsigned id);

    /// Destruct.
    virtual ~IPCWorker();

    void ThreadFunction();

    bool Update();

private:

    // on unix will be the same
    IPCHandle clientRead_;
    IPCHandle clientWrite_;

};

}
