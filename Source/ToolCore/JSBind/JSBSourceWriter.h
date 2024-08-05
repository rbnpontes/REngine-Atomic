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

#pragma once

#include <EngineCore/Container/Str.h>

using namespace Atomic;

namespace ToolCore
{

class JSBSourceWriter
{

public:

    void SetIndentSpaces(int spaces) { indentSpaces_ = spaces; }

    void Indent() { indentLevel_++; GenerateIndentSpaces(); }
    void Dedent() { indentLevel_--; assert(indentLevel_ >= 0); GenerateIndentSpaces(); }

    String IndentLine(const String& line) { return indent_ + line; }

    const String& GetIndent() { return indent_; }

protected:

    void GenerateIndentSpaces() {

        int spaces = indentSpaces_ * indentLevel_;
        indent_.Clear();
        while (spaces--) {
            indent_ += " ";
        }
    }

    String indent_;

    JSBSourceWriter() : indentLevel_(0), indentSpaces_(3) {}
    ~JSBSourceWriter() {}

    int indentSpaces_;
    int indentLevel_;

};

}
