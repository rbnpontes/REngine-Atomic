//
// Copyright (c) 2008-2017 the Urho3D project.
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

#if ENGINE_BINDING_TOOL
    #include "../EngineCore.h"
#endif
// ATOMIC BEGIN
extern "C" {
// ATOMIC END

/// Return true when the running OS has the specified version number or later.
ENGINE_BIND_IGNORE bool CheckMinimalVersion(int major, int minor);

/// Return true when individual file watcher is supported by the running Mac OS X.
ENGINE_BIND_IGNORE bool IsFileWatcherSupported();

/// Create and start the file watcher.
ENGINE_BIND_IGNORE void* CreateFileWatcher(const char* pathname, bool watchSubDirs);

/// Stop and release the file watcher.
ENGINE_BIND_IGNORE void CloseFileWatcher(void* watcher);

/// Read changes queued by the file watcher.
ENGINE_BIND_IGNORE const char* ReadFileWatcher(void* watcher);

// ATOMIC BEGIN
}
// ATOMIC END
