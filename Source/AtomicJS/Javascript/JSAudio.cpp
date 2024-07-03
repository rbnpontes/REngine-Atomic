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

#include "JSAPI.h"
#include "JSAudio.h"

#include <EngineCore/Audio/Sound.h>
#include <EngineCore/Audio/SoundStream.h>

namespace Atomic
{

static int Sound_SetData(duk_context* ctx)
{
    duk_push_this(ctx);
    Sound* sound = js_to_class_instance<Sound>(ctx, -1, 0);
    void* data;
    duk_size_t dataSize;

    if (!js_check_is_buffer_and_get_data(ctx, -2, &data, &dataSize))
    {
        return 0;
    }

    // copy the buffer into the sound
    sound->SetData(data, (unsigned)dataSize);
    return 0;
}

void jsapi_init_audio(JSVM* vm)
{
    duk_context* ctx = vm->GetJSContext();

    js_class_get_prototype(ctx, "Atomic", "Sound");
    duk_push_c_function(ctx, Sound_SetData, 1);
    duk_put_prop_string(ctx, -2, "setData");
    duk_pop(ctx);
}

}
