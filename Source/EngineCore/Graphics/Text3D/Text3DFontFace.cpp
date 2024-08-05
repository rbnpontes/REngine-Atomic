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

#include "../../Precompiled.h"

#include "../../Core/Context.h"
#include "../../IO/Log.h"
#include "../Texture2D.h"
#include "../../Resource/Image.h"
#include "Text3DFont.h"
#include "Text3DFontFace.h"

#include "../../DebugNew.h"

namespace Atomic
{

Text3DFontGlyph::Text3DFontGlyph() :
    page_(M_MAX_UNSIGNED),
    used_(false)
{
}

Text3DFontFace::Text3DFontFace(Text3DFont* font) :
    font_(font)
{
}

Text3DFontFace::~Text3DFontFace()
{
    if (font_)
    {
        // When a face is unloaded, deduct the used texture data size from the parent font
        unsigned totalTextureSize = 0;
        for (unsigned i = 0; i < textures_.Size(); ++i)
            totalTextureSize += textures_[i]->GetWidth() * textures_[i]->GetHeight();
        font_->SetMemoryUse(font_->GetMemoryUse() - totalTextureSize);
    }
}

const Text3DFontGlyph* Text3DFontFace::GetGlyph(unsigned c)
{
    HashMap<unsigned, Text3DFontGlyph>::Iterator i = glyphMapping_.Find(c);
    if (i != glyphMapping_.End())
    {
        Text3DFontGlyph& glyph = i->second_;
        glyph.used_ = true;
        return &glyph;
    }
    else
        return 0;
}

float Text3DFontFace::GetKerning(unsigned c, unsigned d) const
{
    if (kerningMapping_.Empty())
        return 0;

    if (c == '\n' || d == '\n')
        return 0;

    if (c > 0xffff || d > 0xffff)
        return 0;

    unsigned value = (c << 16) + d;

    HashMap<unsigned, float>::ConstIterator i = kerningMapping_.Find(value);
    if (i != kerningMapping_.End())
        return i->second_;

    return 0;
}

bool Text3DFontFace::IsDataLost() const
{
    for (unsigned i = 0; i < textures_.Size(); ++i)
    {
        if (textures_[i]->IsDataLost())
            return true;
    }
    return false;
}


SharedPtr<Texture2D> Text3DFontFace::CreateFaceTexture()
{
    SharedPtr<Texture2D> texture(new Texture2D(font_->GetContext()));
    texture->SetMipsToSkip(QUALITY_LOW, 0); // No quality reduction
    texture->SetNumLevels(1); // No mipmaps
    texture->SetAddressMode(COORD_U, ADDRESS_BORDER);
    texture->SetAddressMode(COORD_V, ADDRESS_BORDER);
    texture->SetBorderColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
    return texture;
}

SharedPtr<Texture2D> Text3DFontFace::LoadFaceTexture(SharedPtr<Image> image)
{
    SharedPtr<Texture2D> texture = CreateFaceTexture();
    if (!texture->SetData(image, true))
    {
        ATOMIC_LOGERROR("Could not load texture from image resource");
        return SharedPtr<Texture2D>();
    }
    return texture;
}

}
