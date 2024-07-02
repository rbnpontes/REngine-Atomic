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

#include <Atomic/Core/Object.h>

namespace Atomic
{


class ATOMIC_API UIDragObject : public Object
{

    ATOMIC_OBJECT(UIDragObject, Object);

public:
    /// Construct.
    UIDragObject(Context* context, Object* object = NULL, const String& text = String::EMPTY, const String& icon = String::EMPTY);
    virtual ~UIDragObject();

    const String& GetText() { return text_; }
    const String& GetIcon() { return icon_; }
    Object* GetObject() { return object_; }

    const Vector<String>& GetFilenames() { return filenames_; }

    void SetText(const String& text) { text_ = text; }
    void SetIcon(const String& icon) { icon_ = icon; }
    void SetObject(Object* object) { object_ = object; }

    void AddFilename(const String& filename) {filenames_.Push(filename); }

private:

    String text_;
    String icon_;
    Vector<String> filenames_;
    SharedPtr<Object> object_;

};


}
